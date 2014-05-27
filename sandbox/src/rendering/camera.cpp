#include "camera.h"
#include "utils/math.h"
#include "utils/logger.h"
#include "utils/stringUtils.h"

namespace sb
{
    Camera::Camera():
        mEye(0.f, 0.f, 0.f),
        mAt(0.f, 0.f, -1.f),
        mUp(0.f, 1.f, 0.f),
        mFront(0.f, 0.f, -1.f),
        mRight(1.f, 0.f, 0.f),
        mUpReal(0.f, 1.f, 0.f),
        mMatrixUpdateFlags(0)
    {
        setOrthographicMatrix();
        setPerspectiveMatrix();

        lookAt(mEye, mAt, mUp);
    }

    void Camera::setOrthographicMatrix(float left,
                                       float right,
                                       float bottom,
                                       float top,
                                       float near,
                                       float far)
    {
        mOrthographicProjectionMatrix =
                math::matrixOrthographic(left, right, bottom, top, near, far);
    }

    void Camera::setPerspectiveMatrix(float fov,
                                      float aspectRatio,
                                      float near,
                                      float far)
    {
        mPerspectiveProjectionMatrix =
                math::matrixPerspective(fov, aspectRatio, near, far);
    }

    void Camera::updateViewMatrix()
    {
        if (mMatrixUpdateFlags & MatrixRotationUpdated) {
            mRotationMatrix = Mat44(
#if 0
                 mRight.x,  mUpReal.x, -mFront.x,  0.f,
                 mRight.y,  mUpReal.y, -mFront.y,  0.f,
                 mRight.z,  mUpReal.z, -mFront.z,  0.f,
                 0.f,       0.f,       0.f,        1.f
#else
                 mRight.x,   mRight.y,   mRight.z,  0.f,
                 mUpReal.x,  mUpReal.y,  mUpReal.z, 0.f,
                -mFront.x,  -mFront.y,  -mFront.z,  0.f,
                 0.f,        0.f,        0.f,       1.f
#endif
            );
        }
        if (mMatrixUpdateFlags & MatrixTranslationUpdated) {
            mTranslationMatrix = glm::translate(-mEye);
        }

        mMatrixUpdateFlags = 0;
        mViewMatrix = mRotationMatrix * mTranslationMatrix;
    }

    // updates only if needed
    Mat44& Camera::getViewMatrix()
    {
        if (mMatrixUpdateFlags) {
            updateViewMatrix();
        }

        return mViewMatrix;
    }

    void Camera::lookAt(Vec3 pos, Vec3 at, Vec3 up)
    {
        mEye = pos;
        mAt = at;
        mUp = up.normalized();

        mFront = (mAt - mEye).normalized();
        mRight = mFront.cross(mUp);     // normalized, since mFront & mUp are normalized
        mUpReal = mRight.cross(mFront); // normalized, since mRight & mFront are normalized

        mMatrixUpdateFlags |= MatrixTranslationUpdated | MatrixRotationUpdated;
    }

    void Camera::rotate(Radians angle)
    {
        Quat rot = glm::angleAxis(angle.value(), mUpReal);

        mFront = rot * mFront;
        mRight = rot * mRight;

        mMatrixUpdateFlags |= MatrixRotationUpdated;
    }

    void Camera::rotate(const Vec3& axis, Radians angle)
    {
        Quat rot = glm::angleAxis(angle.value(), axis.normalized());
        lookAt(mEye, mEye + (rot * (mAt - mEye)), mUp);
    }

    void Camera::rotateAround(Radians angle)
    {
        Quat rot = glm::angleAxis(angle.value(), mUpReal);
        lookAt(mAt + (rot * (mEye - mAt)), mAt, mUp);
    }

    void Camera::mouseLook(Radians dtX, Radians dtY)
    {
        float angleXZ = getHorizontalAngle().value();
        float angleY = getVerticalAngle().value();

        angleXZ += dtX.value();
        angleY += dtY.value();
        angleY = glm::clamp(angleY, -PI_2 + 0.00001f, PI_2 - 0.00001f);

        float len = Vec3(mAt - mEye).length();
        Vec3 at = mEye + Vec3(len * sinf(angleXZ) * cosf(angleY),
                              len * sinf(angleY),
                              len * cosf(angleXZ) * cosf(angleY));

        lookAt(mEye, at, mUp);
    }

    void Camera::move(float distance)
    {
        Vec3 delta = mFront.normalized() * distance;
        mEye += delta;
        mAt += delta;

        mMatrixUpdateFlags |= MatrixTranslationUpdated;
    }

    void Camera::move(const Vec3& delta)
    {
        mEye += delta;
        mAt += delta;

        mMatrixUpdateFlags |= MatrixTranslationUpdated;
    }

    void Camera::strafe(float distance)
    {
        Vec3 delta = mRight.normalized() * distance;
        mEye += delta;
        mAt += delta;

        mMatrixUpdateFlags |= MatrixTranslationUpdated;
    }

    void Camera::ascend(float distance)
    {
        Vec3 delta = mUpReal.normalized() * distance;
        mEye += delta;
        mAt += delta;

        mMatrixUpdateFlags |= MatrixTranslationUpdated;
    }

    // delta = (right, upReal, front) instead of (x, y, z)
    void Camera::moveRelative(const Vec3& delta)
    {
        Vec3 d = mRight.normalized() * delta.x
                 + mUpReal.normalized() * delta.y
                 + mFront.normalized() * delta.z;
        mEye += d;
        mAt += d;

        mMatrixUpdateFlags |= MatrixTranslationUpdated;

    }
}

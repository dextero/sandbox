#include "camera.h"
#include "utils/mathUtils.h"


namespace sb
{
    Camera::Camera():
        mEye(0.f, 0.f, 0.f),
        mAt(0.f, 0.f, -1.f),
        mUp(0.f, 1.f, 0.f),
        mFront(0.f, 0.f, -1.f),
        mRight(1.f, 0.f, 0.f),
        mUpReal(0.f, 1.f, 0.f),
        mAngleXZ(0.f),
        mAngleY(0.f),
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
        if (mMatrixUpdateFlags & MatrixRotationUpdated)
            mRotationMatrix = Mat44(
                 mRight.x,   mRight.y,   mRight.z,  0.f,
                 mUpReal.x,  mUpReal.y,  mUpReal.z, 0.f,
                -mFront.x,  -mFront.y,  -mFront.z,  0.f,
                 0.f,        0.f,        0.f,       1.f
            );
        if (mMatrixUpdateFlags & MatrixTranslationUpdated)
            mTranslationMatrix = glm::translate(-mEye);

        mMatrixUpdateFlags = 0;
        mViewMatrix = mRotationMatrix * mTranslationMatrix;
    }

    // updates only if needed
    Mat44& Camera::getViewMatrix()
    {
        if (mMatrixUpdateFlags)
            updateViewMatrix();

        return mViewMatrix;
    }

    void Camera::lookAt(Vec3 pos, Vec3 at, Vec3 up)
    {
        mEye = pos;
        mAt = at;
        mUp = glm::normalize(up);

        mFront = glm::normalize(Vec3(mAt - mEye));
        mRight = glm::normalize(glm::cross(mFront, mUp));
        mUpReal = glm::normalize(glm::cross(mRight, mFront));

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
        Quat rot = glm::angleAxis(angle.value(), glm::normalize(axis));
        lookAt(mEye, mEye + (rot * (mAt - mEye)), mUp);
    }

    void Camera::rotateAround(Radians angle)
    {
        Quat rot = glm::angleAxis(angle.value(), mUpReal);
        lookAt(mAt + (rot * (mEye - mAt)), mAt, mUp);
    }

    void Camera::mouseLook(Radians dtX, Radians dtY)
    {
        mAngleXZ = Radians(atan2(mFront.z, mFront.x));
        mAngleY = Radians(acos(mFront.y / mFront.length()));

        // don't know why -, but it works
        mAngleXZ = Radians(mAngleXZ.value() - atan(dtX.value()));
        mAngleY = Radians(glm::clamp(mAngleY.value() + atanf(dtY.value()),
                                     0.00001f, PI - 0.00001f));

        float len = Vec3(mAt - mEye).length();
        Vec3 at = mEye + Vec3(len * sinf(mAngleXZ.value())
                                  * sinf(mAngleY.value()),
                              len * cosf(mAngleY.value()),
                              len * cosf(mAngleXZ.value())
                                  * sinf(mAngleY.value()));

        lookAt(mEye, at, mUp);
    }

    void Camera::move(float distance)
    {
        Vec3 delta = glm::normalize(mFront) * distance;
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
        Vec3 delta = glm::normalize(mRight) * distance;
        mEye += delta;
        mAt += delta;

        mMatrixUpdateFlags |= MatrixTranslationUpdated;
    }

    void Camera::ascend(float distance)
    {
        Vec3 delta = glm::normalize(mUpReal) * distance;
        mEye += delta;
        mAt += delta;

        mMatrixUpdateFlags |= MatrixTranslationUpdated;
    }

    // delta = (right, upReal, front) instead of (x, y, z)
    void Camera::moveRelative(const Vec3& delta)
    {
        Vec3 d = glm::normalize(mRight) * delta.x + glm::normalize(mUpReal) * delta.y + glm::normalize(mFront) * delta.z;
        mEye += d;
        mAt += d;

        mMatrixUpdateFlags |= MatrixTranslationUpdated;

    }
}

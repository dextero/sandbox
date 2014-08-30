#include "camera.h"
#include "utils/math.h"
#include "utils/logger.h"
#include "utils/stringUtils.h"
#include "utils/debug.h"

namespace sb
{
    Camera::Camera():
        mProjectionMatrix(),
        mViewMatrix(),
        mRotationMatrix(),
        mTranslationMatrix(),
        mEye(0.f, 0.f, 0.f),
        mAt(0.f, 0.f, -1.f),
        mUp(0.f, 1.f, 0.f),
        mFront(0.f, 0.f, -1.f),
        mRight(1.f, 0.f, 0.f),
        mUpReal(0.f, 1.f, 0.f),
        mXZAngle(0.0),
        mYAngle(0.0),
        mMatrixUpdateFlags(0)
    {
        lookAt(mEye, mAt, mUp);
    }

    Camera Camera::perspective(float fov,
                                float aspectRatio,
                                float near,
                                float far)
    {
        Camera ret;
        ret.mProjectionType = ProjectionType::Perspective;
        ret.mProjectionMatrix = math::matrixPerspective(fov, aspectRatio,
                                                        near, far);
        ret.mPerspectiveFov = fov;
        ret.mPerspectiveNear = near;
        ret.mPerspectiveFar = far;

        return ret;
    }
        
    Camera Camera::orthographic(float left,
                                float right,
                                float bottom,
                                float top,
                                float near,
                                float far)
    {
        Camera ret;
        ret.mProjectionType = ProjectionType::Orthographic;
        ret.mProjectionMatrix = math::matrixOrthographic(left, right,
                                                         bottom, top,
                                                         near, far);
        return ret;
    }

    void Camera::updateViewMatrix()
    {
        if (mMatrixUpdateFlags & MatrixRotationUpdated) {
            mRotationMatrix = Mat44(
#if 1
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
    const Mat44& Camera::getViewMatrix()
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

        Vec3 oldRight = mRight;
        mRight = mFront.cross(mUp);

        if (mRight.isZero()) {
            oldRight.y = 0;
            mRight = oldRight;
            gLog.trace("right was zero, reverted to %s\n", utils::toString(mRight).c_str());
        }

        mRight = mRight.normalized();
        mUpReal = mRight.cross(mFront).normalized();

        sbAssert(mFront.dot(mFront) > 0.0f, "zero front vector");
        sbAssert(mRight.dot(mRight) > 0.0f, "zero right vector");
        sbAssert(mUpReal.dot(mUpReal) > 0.0f, "zero upReal vector");

        sbAssert(std::abs(mRight.length() - 1.0f) < 0.001f,
                 "mRight (%f, %f, %f; length = %f) not normalized",
                 mRight.x, mRight.y, mRight.z, mRight.length());
        sbAssert(std::abs(mUpReal.length() - 1.0f) < 0.001f,
                 "mUpReal (%f, %f, %f; length = %f) not normalized",
                 mUpReal.x, mUpReal.y, mUpReal.z, mUpReal.length());
        sbAssert(std::abs(mFront.length() - 1.0f) < 0.001f,
                 "mFront (%f, %f, %f; length = %f) not normalized",
                 mFront.x, mFront.y, mFront.z, mFront.length());

        mMatrixUpdateFlags |= MatrixTranslationUpdated | MatrixRotationUpdated;
        updateAngles();
    }

    void Camera::rotate(Radians angle)
    {
        Quat rot = glm::angleAxis(angle.value(), mUpReal);

        mFront = Vec3(rot * mFront).normalized();
        mRight = Vec3(rot * mRight).normalized();

        mMatrixUpdateFlags |= MatrixRotationUpdated;
        updateAngles();
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

        angleXZ -= dtX.value(); // -? lol
        angleY -= dtY.value();  // same
        angleY = glm::clamp(angleY, -PI_2, PI_2);

        float len = Vec3(mAt - mEye).length();
        Vec3 at = mEye + Vec3(len * sinf(angleXZ) * cosf(angleY),
                              len * sinf(angleY),
                              len * cosf(angleXZ) * cosf(angleY));

        //gLog.trace("%f, %f, len %f\n", angleXZ, angleY, len);
        //gLog.trace("eye: %s\n", utils::toString(mEye).c_str());
        //gLog.trace("at: %s\n", utils::toString(mAt).c_str());
        //gLog.trace("at - eye: %s\n", utils::toString(Vec3(mAt - mEye)).c_str());
        //gLog.trace("new at = %s\n", utils::toString(Vec3(len * sinf(angleXZ) * cosf(angleY),
                                                     //len * sinf(angleY),
                                                     //len * cosf(angleXZ) * cosf(angleY))).c_str());

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

    void Camera::updateViewport(unsigned width,
                                unsigned height)
    {
        if (mProjectionType == ProjectionType::Perspective) {
            mProjectionMatrix = math::matrixPerspective(
                    mPerspectiveFov, (float)width / (float)height, 
                    mPerspectiveNear, mPerspectiveFar);
        } else {
            mProjectionMatrix = math::matrixOrthographic(
                    0.0f, (float)width, (float)height, 0.0f, -100.0f, 100.0f);
        }
    }

    void Camera::updateAngles()
    {
        if (std::abs(mFront.x) >= math::EPSILON
                || std::abs(mFront.z) >= math::EPSILON) {
            mXZAngle = Radians(std::atan2(mFront.x, mFront.z));
        }

        mYAngle = Radians(std::asin(mFront.y / mFront.length()));
    }
}

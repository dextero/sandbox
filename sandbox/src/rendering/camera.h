#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>

#include "types.h"
#include "utils/math.h"
#include "utils/types.h"

namespace sb
{
    class Renderer;

    class Camera
    {
    public:
        Camera();

        void setOrthographicMatrix(float left = -1.f,
                                   float right = 1.f,
                                   float bottom = -1.f,
                                   float top = 1.f,
                                   float near = -1.f,
                                   float far = 1.f);
        void setPerspectiveMatrix(float fov = PI_3,
                                  float aspectRatio = 1.33f,
                                  float near = Z_NEAR,
                                  float far = Z_FAR);
        void updateViewMatrix();

        Mat44& getOrthographicProjectionMatrix()
        {
            return mOrthographicProjectionMatrix;
        }
        Mat44& getPerspectiveProjectionMatrix()
        {
            return mPerspectiveProjectionMatrix;
        }
        Mat44& getViewMatrix();    // updates only if needed

        Mat44 getViewProjectionMatrix(EProjectionType projectionType)
        {
            if (projectionType == ProjectionOrthographic) {
                return getOrthographicProjectionMatrix();
            }

            return getPerspectiveProjectionMatrix() * getViewMatrix();
        }

        void lookAt(Vec3 pos,
                    Vec3 at,
                    Vec3 up = Vec3(0.f, 1.f, 0.f));
        void rotate(Radians angle);
        void rotate(const Vec3& axis,
                    Radians angle);
        void rotateAround(Radians angle);
        void mouseLook(Radians dtX,
                       Radians dtY);

        void move(float distance);
        void move(const Vec3& delta);
        void strafe(float distance);
        void ascend(float distance);
        void moveRelative(const Vec3& delta); // delta = (right, upReal, front) instead of (x, y, z)

        const Vec3& getEye() { return mEye; }
        const Vec3& getAt() { return mAt; }
        const Vec3& getUp() { return mUp; }
        const Vec3& getFront() { return mFront; }
        const Vec3& getRight() { return mRight; }
        const Vec3& getUpReal() { return mUpReal; }
        Radians getHorizontalAngle() { return mXZAngle; }
        Radians getVerticalAngle() { return mYAngle; }

    private:
        Mat44 mOrthographicProjectionMatrix;
        Mat44 mPerspectiveProjectionMatrix;
        Mat44 mViewMatrix;

        Mat44 mRotationMatrix;
        Mat44 mTranslationMatrix;

        Vec3 mEye;
        Vec3 mAt;
        Vec3 mUp;
        Vec3 mFront;
        Vec3 mRight;
        Vec3 mUpReal;

        Radians mXZAngle;
        Radians mYAngle;

        enum EMatrixUpdateFlags {
            MatrixRotationUpdated = 1,
            MatrixTranslationUpdated = 1 << 1
        };
        uint32_t mMatrixUpdateFlags;

        // needs to be called after every mFront change
        void updateAngles();
    };
} // namespace sb

#endif //CAMERA_H

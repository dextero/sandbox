#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>

#include <sandbox/rendering/types.h>
#include <sandbox/utils/math.h>
#include <sandbox/utils/types.h>

namespace sb
{
    class Renderer;

    class Camera
    {
    public:
        static Camera perspective(float fov = PI_3,
                                  float aspectRatio = 1.33f,
                                  float near = Z_NEAR,
                                  float far = Z_FAR);

        static Camera orthographic(float left = -1.f,
                                   float right = 1.f,
                                   float bottom = -1.f,
                                   float top = 1.f,
                                   float near = -1.f,
                                   float far = 1.f);
        void updateViewMatrix();

        const Mat44& getProjectionMatrix() const
        {
            return mProjectionMatrix;
        }
        const Mat44& getViewMatrix();    // updates only if needed

        Mat44 getViewProjectionMatrix()
        {
            return getProjectionMatrix() * getViewMatrix();
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

        const Vec3& getEye() const { return mEye; }
        const Vec3& getAt() const { return mAt; }
        const Vec3& getUp() const { return mUp; }
        const Vec3& getFront() const { return mFront; }
        const Vec3& getRight() const { return mRight; }
        const Vec3& getUpReal() const { return mUpReal; }
        Radians getHorizontalAngle() const { return mXZAngle; }
        Radians getVerticalAngle() const { return mYAngle; }

        void updateViewport(unsigned width,
                            unsigned height);

    private:
        float mPerspectiveFov;
        float mPerspectiveNear;
        float mPerspectiveFar;

        ProjectionType mProjectionType;
        Mat44 mProjectionMatrix;
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

        Camera();

        // needs to be called after every mFront change
        void updateAngles();
    };
} // namespace sb

#endif //CAMERA_H

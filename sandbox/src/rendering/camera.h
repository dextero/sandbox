#ifndef CAMERA_H
#define CAMERA_H

#include "types.h"
#include "../utils/mathUtils.h"
#include "../utils/types.h"
#include "../utils/profiler.h"

namespace sb
{
	class Renderer;
	
	class Camera
	{
		Mat44 mOrthographicProjectionMatrix,
			  mPerspectiveProjectionMatrix,
			  mViewMatrix;
	
		Mat44 mRotationMatrix, mTranslationMatrix;
		Vec3 mEye, mAt, mUp, mFront, mRight, mUpReal;
		float mAngleXZ, mAngleY;
	
		enum EMatrixUpdateFlags {
			MatrixRotationUpdated = 1,
			MatrixTranslationUpdated = 1 << 1
		};
		uint mMatrixUpdateFlags;
	
	public:
		Camera();
	
		void SetOrthographicMatrix(float left = -1.f, float right = 1.f, float bottom = -1.f, float top = 1.f, float near = -1.f, float far = 1.f);
		void SetPerspectiveMatrix(float fov = PI_3, float aspectRatio = 1.33f, float near = Z_NEAR, float far = Z_FAR);
		void UpdateViewMatrix();
	
		Mat44& GetOrthographicProjectionMatrix();
		Mat44& GetPerspectiveProjectionMatrix();
		Mat44& GetViewMatrix();	// updates only if needed
	
		void LookAt(Vec3 pos, Vec3 at, Vec3 up = Vec3(0.f, 1.f, 0.f));
		void Rotate(float angle);
		void Rotate(const Vec3& axis, float angle);
		void RotateAround(float angle);
		void MouseLook(float dtX, float dtY);
	
		void Move(float distance);
		void Move(const Vec3& delta);
		void Strafe(float distance);
		void Ascend(float distance);
		void MoveRelative(const Vec3& delta); // delta = (right, upReal, front) instead of (x, y, z)
	
		const Vec3& GetEye();
		const Vec3& GetAt();
		const Vec3& GetUp();
		const Vec3& GetFront();
		const Vec3& GetRight();
		const Vec3& GetUpReal();
		float GetHorizontalAngle();
		float GetVerticalAngle();
	};
} // namespace sb

#endif //CAMERA_H
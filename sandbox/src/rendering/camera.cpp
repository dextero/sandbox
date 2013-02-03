#include "camera.h"


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
		PROFILE();
	
		SetOrthographicMatrix();
		SetPerspectiveMatrix();
		mViewMatrix.identity();
	
		LookAt(mEye, mAt, mUp);
	}
	
	void Camera::SetOrthographicMatrix(float left, float right, float bottom, float top, float near, float far)
	{
		PROFILE();
	
		float r_min_l = right - left;
		float t_min_b = top - bottom;
		float f_min_n = far - near;
	
		mOrthographicProjectionMatrix.set(
			2.f / r_min_l, 0, 0, -(right + left) / r_min_l,
			0, 2.f / t_min_b, 0, -(top + bottom) / t_min_b,
			0, 0, -2 / f_min_n, -(far + near) / f_min_n,
			0, 0, 0, 1
		);
	}
	
	void Camera::SetPerspectiveMatrix(float fov, float aspectRatio, float near, float far)
	{
		PROFILE();
	
		float xymax = near * tan(fov / 2.f);
		float ymin = -xymax;
		float xmin = -xymax;
		
		float width = xymax - xmin;
		float height = xymax - ymin;
		
		float depth = far - near;
		float q = -(far + near) / depth;
		float qn = -2 * (far * near) / depth;
		
		float w = 2 * near / width;
		w = w / aspectRatio;
		float h = 2 * near / height;
	
		mPerspectiveProjectionMatrix.set(
			w, 0, 0, 0,
			0, h, 0, 0,
			0, 0, q, qn,
			0, 0, -1, 0
		);
	}
	
	void Camera::UpdateViewMatrix()
	{
		PROFILE();
	
		if (mMatrixUpdateFlags & MatrixRotationUpdated)
			mRotationMatrix.set(
				/*mRight[0],	mUpReal[0],	-mFront[0], 0.f,
				mRight[1],	mUpReal[1],	-mFront[1], 0.f,
				mRight[2],	mUpReal[2],	-mFront[2], 0.f,
				0.f,		0.f,		0.f,		1.f*/
				mRight[0],	mRight[1],	mRight[2],	0.f,
				mUpReal[0],	mUpReal[1],	mUpReal[2],	0.f,
				-mFront[0],	-mFront[1],	-mFront[2],	0.f,
				0.f,		0.f,		0.f,		1.f
			);
		if (mMatrixUpdateFlags & MatrixTranslationUpdated)
			cml::matrix_translation(mTranslationMatrix, -mEye);
	
		mMatrixUpdateFlags = 0;
		mViewMatrix = mRotationMatrix * mTranslationMatrix;
	}
	
	Mat44& Camera::GetOrthographicProjectionMatrix()
	{
		PROFILE();
	
		return mOrthographicProjectionMatrix;
	}
	
	Mat44& Camera::GetPerspectiveProjectionMatrix()
	{
		PROFILE();
	
		return mPerspectiveProjectionMatrix;
	}
	
	// updates only if needed
	Mat44& Camera::GetViewMatrix()
	{
		PROFILE();
	
		if (mMatrixUpdateFlags)
			UpdateViewMatrix();
	
		return mViewMatrix;
	}
	
	void Camera::LookAt(Vec3 pos, Vec3 at, Vec3 up)
	{
		PROFILE();
	
		mEye = pos;
		mAt = at;
		mUp = up.normalized();
	
		mFront = Vec3(mAt - mEye).normalized();
		mRight = cml::cross(mFront, mUp).normalized();
		//if (mRight != mRight) // hgw
			//mRight = cml::cross(mFront, mUpReal);
	
		mUpReal = cml::cross(mRight, mFront).normalized();
	
		//mAngleXZ = cml::asin_safe(mFront[2] / mFront.length());
		//mAngleY = atanf(mFront[1] / mFront[0]);
	
		mMatrixUpdateFlags |= MatrixTranslationUpdated | MatrixRotationUpdated;
	}
	
	void Camera::Rotate(float angle)
	{
		PROFILE();
	
		Mat33 rotMat;
		cml::matrix_rotation_axis_angle(rotMat, mUpReal, angle);
	
		mFront = rotMat * mFront;
		mRight = rotMat * mRight;
	
		mMatrixUpdateFlags |= MatrixRotationUpdated;
	}
	
	void Camera::Rotate(const Vec3& axis, float angle)
	{
		PROFILE();
	
		Mat33 rotMat;
		cml::matrix_rotation_axis_angle(rotMat, axis.normalized(), angle);
		LookAt(mEye, mEye + (rotMat * (mAt - mEye)), mUp);
	}
	
	void Camera::RotateAround(float angle)
	{
		PROFILE();
	
		Mat33 rotMat;
		cml::matrix_rotation_axis_angle(rotMat, mUpReal, angle);
		LookAt(mAt + (rotMat * (mEye - mAt)), mAt, mUp);
	}
	
	void Camera::MouseLook(float dtX, float dtY)
	{
		PROFILE();
	
		mAngleXZ = atan(mFront[0] / mFront[2]) + cml::constantsf::pi() * (float)(mFront[2] < 0.f);
		mAngleY = acos(mFront[1] / mFront.length());
	
		// don't know why -, but it works
		mAngleXZ = mAngleXZ - atan(dtX);
		mAngleY = cml::clamp(mAngleY + atanf(dtY), 0.00001f, cml::constantsf::pi() - 0.00001f);
	
		float len = Vec3(mAt - mEye).length();
		Vec3 at = mEye + Vec3(len * sinf(mAngleXZ) * sinf(mAngleY), len * cosf(mAngleY), len * cosf(mAngleXZ) * sinf(mAngleY));
	
		LookAt(mEye, at, mUp);
	}
	
	void Camera::Move(float distance)
	{
		PROFILE();
	
		Vec3 delta = mFront.normalize() * distance;
		mEye += delta;
		mAt += delta;
	
		mMatrixUpdateFlags |= MatrixTranslationUpdated;
	}
	
	void Camera::Move(const Vec3& delta)
	{
		PROFILE();
	
		mEye += delta;
		mAt += delta;
	
		mMatrixUpdateFlags |= MatrixTranslationUpdated;
	}
	
	void Camera::Strafe(float distance)
	{
		PROFILE();
	
		Vec3 delta = mRight.normalize() * distance;
		mEye += delta;
		mAt += delta;
	
		mMatrixUpdateFlags |= MatrixTranslationUpdated;
	}
	
	void Camera::Ascend(float distance)
	{
		PROFILE();
	
		Vec3 delta = mUpReal.normalize() * distance;
		mEye += delta;
		mAt += delta;
	
		mMatrixUpdateFlags |= MatrixTranslationUpdated;
	}
	
	// delta = (right, upReal, front) instead of (x, y, z)
	void Camera::MoveRelative(const Vec3& delta)
	{
		PROFILE();
	
		Vec3 d = mRight.normalize() * delta[0] + mUpReal.normalize() * delta[1] + mFront.normalize() * delta[2];
		mEye += d;
		mAt += d;
	
		mMatrixUpdateFlags |= MatrixTranslationUpdated;
	
	}
	
	const Vec3& Camera::GetEye()
	{
		PROFILE();
	
		return mEye;
	}
	
	const Vec3& Camera::GetAt()
	{
		PROFILE();
	
		return mAt;
	}
	
	const Vec3& Camera::GetUp()
	{
		PROFILE();
	
		return mUp;
	}
	
	const Vec3& Camera::GetFront()
	{
		PROFILE();
	
		return mFront;
	}
	
	const Vec3& Camera::GetRight()
	{
		PROFILE();
	
		return mRight;
	}
	
	const Vec3& Camera::GetUpReal()
	{
		PROFILE();
	
		return mUpReal;
	}
	
	float Camera::GetHorizontalAngle()
	{
		PROFILE();
		
		return mAngleXZ;
	}
	
	float Camera::GetVerticalAngle()
	{
		PROFILE();
	
		return mAngleY;
	}
}

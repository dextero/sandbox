#include "drawable.h"
#include "shader.h"

#include "resources/mesh.h"
#include "resources/image.h"

#include "utils/profiler.h"

namespace sb
{
	Drawable::Drawable(EProjectionType projType):
		mMesh(NULL),
		mTexture(0),
		mColor(Color::White),
		mFlags(0),
		mPosition(0.f, 0.f, 0.f),
		mScale(1.f, 1.f, 1.f),
		mProjectionType(projType)
	{
		PROFILE();
	
		mTransformationMatrix.identity();
		mTranslationMatrix.identity();
		mScaleMatrix.identity();
		mRotationMatrix.identity();
		mRotation.identity();
	}
	
	void Drawable::RecalculateMatrices()
	{
		PROFILE();
	
		if (mFlags & FlagTransformationChanged)
		{
			mTransformationMatrix.identity();
	
			if (mFlags & FlagScaleChanged)
				cml::matrix_scale(mScaleMatrix, mScale);
			if (mFlags & FlagRotationChanged)
				cml::matrix_rotation_quaternion(mRotationMatrix, mRotation);
			if (mFlags & FlagPositionChanged)
				cml::matrix_translation(mTranslationMatrix, mPosition);
	
			mTransformationMatrix = mTranslationMatrix * mRotationMatrix * mScaleMatrix;
			mFlags &= ~FlagTransformationChanged;
		}
	}
	
	Drawable::Drawable():
		mTexture(0),
		mMesh(NULL)
	{
	}
	
	Drawable::~Drawable()
	{
		PROFILE();
	
		if (mMesh)
			gResourceMgr.FreeMesh(mMesh);
		if (mTexture)
			gResourceMgr.FreeTexture(mTexture);
	}
	
	Drawable::Drawable(const Drawable& copy)
	{
		PROFILE();
	
		*this = copy;
	
		if (copy.mMesh)
			mMesh = gResourceMgr.GetMesh(copy.mMesh);
		if (copy.mTexture)
			mTexture = gResourceMgr.GetTexture(copy.mTexture);
	}
	
	const Vec3& Drawable::GetPosition() const
	{
		PROFILE();
		return mPosition;
	}
	
	const Vec3 Drawable::GetRotationAxis() const
	{
		PROFILE();
		
		Vec3 axis;
		float angle;
		cml::quaternion_to_axis_angle(mRotation, axis, angle);
		return axis;
	}
	
	float Drawable::GetRotationAngle() const
	{
		PROFILE();
		
		Vec3 axis;
		float angle;
		cml::quaternion_to_axis_angle(mRotation, axis, angle);
		return angle;
	}
	void Drawable::GetRotationAxisAngle(Vec3& axis, float& angle) const
	{
		PROFILE();
		
		cml::quaternion_to_axis_angle(mRotation, axis, angle);
	}
	
	const Quat& Drawable::GetRotationQuaternion() const
	{
		PROFILE();
		
		return mRotation;
	}
	
	const Vec3& Drawable::GetScale() const
	{
		PROFILE();
		
		return mScale;
	}
	
	Shader::EShader Drawable::GetShader() const
	{
		if (mTexture || (mMesh && mMesh->GetTexture()))
			return Shader::ShaderTexture;
		else
			return Shader::ShaderColor;
	}
	
	void Drawable::SetPosition(const Vec3& pos)
	{
		PROFILE();
		
		mPosition = pos;
		mFlags |= FlagPositionChanged;
	}
	
	void Drawable::SetPosition(float x, float y, float z)
	{
		PROFILE();
		
		mPosition = Vec3(x, y, z);
		mFlags |= FlagPositionChanged;
	}
	
	void Drawable::SetRotation(const Vec3& axis, float angle)
	{
		PROFILE();
		
		cml::quaternion_rotation_axis_angle(mRotation, axis.normalized(), angle);
		mFlags |= FlagRotationChanged;
	}
	
	void Drawable::SetRotation(float x, float y, float z)
	{
		PROFILE();
		
		cml::quaternion_rotation_euler(mRotation, x, y, z, cml::euler_order_xyz);
		mFlags |= FlagRotationChanged;
	}
	
	void Drawable::SetScale(const Vec3& scale)
	{
		PROFILE();
		
		mScale = scale;
		mFlags |= FlagScaleChanged;
	}
	
	void Drawable::SetScale(float x, float y, float z)
	{
		PROFILE();
		
		mScale = Vec3(x, y, z);
		mFlags |= FlagScaleChanged;
	}
	
	void Drawable::SetScale(float uniform)
	{
		PROFILE();
		
		mScale = Vec3(uniform, uniform, uniform);
		mFlags |= FlagScaleChanged;
	}
	
	void Drawable::Rotate(float angle)
	{
		PROFILE();
	
		Vec3 axis;
		float prevAngle;
		cml::quaternion_to_axis_angle(mRotation, axis, prevAngle);
		cml::quaternion_rotation_axis_angle(mRotation, axis, fmodf(prevAngle + angle, PI_2));
	
		mFlags |= FlagRotationChanged;
	}
	
	void Drawable::Rotate(const Vec3& axis, float angle)
	{
		PROFILE();
	
		Quat q;
		cml::quaternion_rotation_axis_angle(q, axis.normalized(), angle);
		mRotation *= q;
		
		mFlags |= FlagRotationChanged;
	}
	
	const Mat44& Drawable::GetTransformationMatrix()
	{
		PROFILE();
	
		RecalculateMatrices();
		return mTransformationMatrix;
	}
}

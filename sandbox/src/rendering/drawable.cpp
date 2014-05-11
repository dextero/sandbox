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
    {}

    void Drawable::RecalculateMatrices()
    {
        if (mFlags & FlagTransformationChanged)
        {
            mTransformationMatrix = Mat44();

            if (mFlags & FlagScaleChanged)
                mScaleMatrix = glm::scale(mScale);
            if (mFlags & FlagRotationChanged)
                mRotationMatrix = glm::toMat4(mRotation);
            if (mFlags & FlagPositionChanged)
                mTranslationMatrix = glm::translate(mPosition);

            mTransformationMatrix = mTranslationMatrix
                                    * mRotationMatrix
                                    * mScaleMatrix;
            mFlags &= ~FlagTransformationChanged;
        }
    }

    Drawable::Drawable():
        mMesh(NULL),
        mTexture(0)
    {
    }

    Drawable::~Drawable()
    {
        if (mMesh)
            gResourceMgr.FreeMesh(mMesh);
        if (mTexture)
            gResourceMgr.FreeTexture(mTexture);
    }

    Drawable::Drawable(const Drawable& copy)
    {
        *this = copy;

        if (copy.mMesh)
            mMesh = gResourceMgr.GetMesh(copy.mMesh);
        if (copy.mTexture)
            mTexture = gResourceMgr.GetTexture(copy.mTexture);
    }

    const Vec3& Drawable::GetPosition() const
    {
        return mPosition;
    }

    const Vec3 Drawable::GetRotationAxis() const
    {
        return glm::axis(mRotation);
    }

    Radians Drawable::GetRotationAngle() const
    {
        return Radians(glm::angle(mRotation));
    }
    void Drawable::GetRotationAxisAngle(Vec3& axis, Radians& angle) const
    {
        axis = GetRotationAxis();
        angle = GetRotationAngle();
    }

    const Quat& Drawable::GetRotationQuaternion() const
    {
        return mRotation;
    }

    const Vec3& Drawable::GetScale() const
    {
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
        mPosition = pos;
        mFlags |= FlagPositionChanged;
    }

    void Drawable::SetPosition(float x, float y, float z)
    {
        mPosition = Vec3(x, y, z);
        mFlags |= FlagPositionChanged;
    }

    void Drawable::SetRotation(const Vec3& axis, Radians angle)
    {
        mRotation = glm::angleAxis(angle.value(), glm::normalize(axis));
        mFlags |= FlagRotationChanged;
    }

    void Drawable::SetRotation(Radians x, Radians y, Radians z)
    {
        mRotation = glm::toQuat(glm::eulerAngleYXZ(y.value(),
                                                   x.value(),
                                                   z.value()));
        mFlags |= FlagRotationChanged;
    }

    void Drawable::SetScale(const Vec3& scale)
    {
        mScale = scale;
        mFlags |= FlagScaleChanged;
    }

    void Drawable::SetScale(float x, float y, float z)
    {
        mScale = Vec3(x, y, z);
        mFlags |= FlagScaleChanged;
    }

    void Drawable::SetScale(float uniform)
    {
        mScale = Vec3(uniform, uniform, uniform);
        mFlags |= FlagScaleChanged;
    }

    void Drawable::Rotate(Radians angle)
    {
        mRotation = glm::rotate(mRotation, angle.value(), GetRotationAxis());
        mFlags |= FlagRotationChanged;
    }

    void Drawable::Rotate(const Vec3& axis, Radians angle)
    {
        mRotation = glm::rotate(mRotation, angle.value(), axis);
        mFlags |= FlagRotationChanged;
    }

    const Mat44& Drawable::GetTransformationMatrix()
    {
        RecalculateMatrices();
        return mTransformationMatrix;
    }
}

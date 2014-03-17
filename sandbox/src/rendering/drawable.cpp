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
    }

    void Drawable::RecalculateMatrices()
    {
        PROFILE();

        if (mFlags & FlagTransformationChanged)
        {
            mTransformationMatrix = Mat44();

            if (mFlags & FlagScaleChanged)
                mScaleMatrix = glm::scale(mScale);
            if (mFlags & FlagRotationChanged)
                mRotationMatrix = glm::toMat4(mRotation);
            if (mFlags & FlagPositionChanged)
                mTranslationMatrix = glm::translate(mPosition);

            mTransformationMatrix = mTranslationMatrix * mRotationMatrix * mScaleMatrix;
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
        return glm::axis(mRotation);
    }

    float Drawable::GetRotationAngle() const
    {
        PROFILE();
        return glm::angle(mRotation);
    }
    void Drawable::GetRotationAxisAngle(Vec3& axis, float& angle) const
    {
        PROFILE();

        axis = GetRotationAxis();
        angle = GetRotationAngle();
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
        mRotation = glm::angleAxis(angle, glm::normalize(axis));
        mFlags |= FlagRotationChanged;
    }

    void Drawable::SetRotation(float x, float y, float z)
    {
        PROFILE();

        mRotation = glm::toQuat(glm::eulerAngleYXZ(y, x, z));
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

        mRotation = glm::rotate(mRotation, angle, GetRotationAxis());
        mFlags |= FlagRotationChanged;
    }

    void Drawable::Rotate(const Vec3& axis, float angle)
    {
        PROFILE();

        mRotation = glm::rotate(mRotation, angle, axis);
        mFlags |= FlagRotationChanged;
    }

    const Mat44& Drawable::GetTransformationMatrix()
    {
        PROFILE();

        RecalculateMatrices();
        return mTransformationMatrix;
    }
}

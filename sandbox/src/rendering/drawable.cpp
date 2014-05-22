#include "drawable.h"
#include "shader.h"

#include "resources/mesh.h"
#include "resources/image.h"

namespace sb
{
    Drawable::Drawable(EProjectionType projType):
        mColor(Color::White),
        mFlags(0),
        mPosition(0.f, 0.f, 0.f),
        mScale(1.f, 1.f, 1.f),
        mProjectionType(projType)
    {}

    void Drawable::recalculateMatrices()
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

    Drawable::Drawable()
    {
    }

    const Vec3& Drawable::getPosition() const
    {
        return mPosition;
    }

    const Vec3 Drawable::getRotationAxis() const
    {
        return glm::axis(mRotation);
    }

    Radians Drawable::getRotationAngle() const
    {
        return Radians(glm::angle(mRotation));
    }
    void Drawable::getRotationAxisAngle(Vec3& axis, Radians& angle) const
    {
        axis = getRotationAxis();
        angle = getRotationAngle();
    }

    const Quat& Drawable::getRotationQuaternion() const
    {
        return mRotation;
    }

    const Vec3& Drawable::getScale() const
    {
        return mScale;
    }

    Shader::EShader Drawable::getShader() const
    {
        if (mTexture || (mMesh && mMesh->getTexture())) {
            return Shader::ShaderTexture;
        }
        return Shader::ShaderColor;
    }

    void Drawable::setPosition(const Vec3& pos)
    {
        mPosition = pos;
        mFlags |= FlagPositionChanged;
    }

    void Drawable::setPosition(float x, float y, float z)
    {
        mPosition = Vec3(x, y, z);
        mFlags |= FlagPositionChanged;
    }

    void Drawable::setRotation(const Vec3& axis, Radians angle)
    {
        mRotation = glm::angleAxis(angle.value(), glm::normalize(axis));
        mFlags |= FlagRotationChanged;
    }

    void Drawable::setRotation(Radians x, Radians y, Radians z)
    {
        mRotation = glm::toQuat(glm::eulerAngleYXZ(y.value(),
                                                   x.value(),
                                                   z.value()));
        mFlags |= FlagRotationChanged;
    }

    void Drawable::setScale(const Vec3& scale)
    {
        mScale = scale;
        mFlags |= FlagScaleChanged;
    }

    void Drawable::setScale(float x, float y, float z)
    {
        mScale = Vec3(x, y, z);
        mFlags |= FlagScaleChanged;
    }

    void Drawable::setScale(float uniform)
    {
        mScale = Vec3(uniform, uniform, uniform);
        mFlags |= FlagScaleChanged;
    }

    void Drawable::rotate(Radians angle)
    {
        mRotation = glm::rotate(mRotation, angle.value(), getRotationAxis());
        mFlags |= FlagRotationChanged;
    }

    void Drawable::rotate(const Vec3& axis, Radians angle)
    {
        mRotation = glm::rotate(mRotation, angle.value(), axis);
        mFlags |= FlagRotationChanged;
    }

    const Mat44& Drawable::getTransformationMatrix()
    {
        recalculateMatrices();
        return mTransformationMatrix;
    }
}

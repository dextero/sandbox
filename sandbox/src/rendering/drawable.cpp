#include <cassert>

#include "drawable.h"
#include "shader.h"

#include "resources/mesh.h"
#include "resources/image.h"

namespace sb
{
    Drawable::Drawable(EProjectionType projType,
                       const std::shared_ptr<Mesh>& mesh,
                       const std::shared_ptr<Texture>& texture,
                       const std::shared_ptr<Shader>& shader):
        mMesh(mesh),
        mTexture(texture ? texture : gResourceMgr.getDefaultTexture()),
        mShader(shader),
        mColor(Color::White),
        mFlags(0),
        mPosition(0.f, 0.f, 0.f),
        mScale(1.f, 1.f, 1.f),
        mProjectionType(projType)
    {}

    void Drawable::recalculateMatrices() const
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
        mRotation = glm::angleAxis(angle.value(), axis.normalized());
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

    const Mat44& Drawable::getTransformationMatrix() const
    {
        recalculateMatrices();
        return mTransformationMatrix;
    }

    void Drawable::draw(Renderer::State& state) const
    {
        if (mShader != state.shader) {
            state.shader = mShader;

            assert(state.shader != nullptr);

            state.shader->bind();
            state.shader->setUniform("texture", (int)Shader::SamplerImage);
        }

        if (mTexture != state.texture) {
            state.texture = mTexture;

            assert(state.texture != nullptr);

            state.texture->bind(0);
        }

        state.shader->setUniform("matViewProjection",
                state.camera.getViewProjectionMatrix(mProjectionType));

        state.shader->setUniform("matModel", getTransformationMatrix());
        state.shader->setUniform("color", mColor);

        auto bind = make_bind(mMesh->getVertexBuffer());
        auto indexBind = make_bind(mMesh->getIndexBuffer());

        GL_CHECK(glDrawElements(mMesh->getShape(),
                                mMesh->getIndexBufferSize(),
                                GL_UNSIGNED_INT, NULL));
    }
}

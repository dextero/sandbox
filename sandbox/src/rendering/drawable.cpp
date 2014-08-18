#include "drawable.h"
#include "shader.h"

#include "resources/mesh.h"
#include "resources/image.h"

namespace sb {

Drawable::Drawable(EProjectionType projType,
                   const std::shared_ptr<Mesh>& mesh,
                   const std::shared_ptr<Texture>& texture,
                   const std::shared_ptr<Shader>& shader):
    mMesh(mesh),
    mTexture(texture ? texture : gResourceMgr.getDefaultTexture()),
    mShader(shader),
    mColor(Color::White),
    mTranslationMatrix(),
    mScaleMatrix(),
    mRotationMatrix(),
    mTransformationMatrix(),
    mFlags(0),
    mPosition(0.f, 0.f, 0.f),
    mScale(1.f, 1.f, 1.f),
    mRotation(),
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

bool Drawable::operator <(const Drawable& d) const {
    int diff = mShader.get() - d.mShader.get();
    if (diff) {
        return diff < 0;
    }

    diff = mTexture.get() - d.mTexture.get();
    if (diff) {
        return diff < 0;
    }

    float zDiff = (mRotation * mPosition).z
                  - (d.mRotation * mPosition).z;
    return zDiff < 0;
}

namespace {

void setLightUniforms(const Renderer::State& state,
                      const std::shared_ptr<Shader>& shader)
{
    if (shader->hasUniform("ambientLightColor")) {
        shader->setUniform("ambientLightColor", state.ambientLightColor);
    }

    sbAssert(shader->hasUniform("pointLights") == shader->hasUniform("numPointLights"),
             "pointLights and numPointLights must both be present");

    if (shader->hasUniform("pointLights")) {
        shader->setUniform("numPointLights", (unsigned)state.pointLights.size());
        for (size_t i = 0; i < state.pointLights.size(); ++i) {
            std::string base = utils::format("pointLights[{0}]", i);
            const Light& l = state.pointLights[i];

            shader->setUniform(base + ".position", l.pos);
            shader->setUniform(base + ".color", l.color);
            shader->setUniform(base + ".intensity", l.intensity);
        }
    }

    sbAssert(shader->hasUniform("parallelLights") == shader->hasUniform("numParallelLights"),
             "parallelLights and numParallelLights must both be present");

    if (shader->hasUniform("parallelLights")) {
        shader->setUniform("numParallelLights", (unsigned)state.parallelLights.size());
        for (size_t i = 0; i < state.parallelLights.size(); ++i) {
            std::string base = utils::format("parallelLights[{0}]", i);
            const Light& l = state.parallelLights[i];

            shader->setUniform(base + ".direction", l.pos);
            shader->setUniform(base + ".color", l.color);
            shader->setUniform(base + ".intensity", l.intensity);
        }
    }
}

} // namespace

void Drawable::draw(Renderer::State& state) const
{
    auto vaoBind = make_bind(mMesh->getVertexBuffer());
    auto indexBind = make_bind(mMesh->getIndexBuffer());
    auto shaderBind = make_bind(*mShader, mMesh->getVertexBuffer());
    auto textureBind = make_bind(*mTexture, 0);

    mShader->setUniform("texture", 0);
    mShader->setUniform("matViewProjection",
            state.camera.getViewProjectionMatrix(mProjectionType));
    mShader->setUniform("matModel", getTransformationMatrix());
    mShader->setUniform("color", mColor);

    setLightUniforms(state, mShader);

    GL_CHECK(glDrawElements((GLuint)mMesh->getShape(),
                            mMesh->getIndexBufferSize(),
                            GL_UNSIGNED_INT, (void*)NULL));
}

}

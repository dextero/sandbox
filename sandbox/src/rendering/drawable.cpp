#include "drawable.h"
#include "shader.h"

#include "resources/mesh.h"
#include "resources/image.h"

namespace sb {

Drawable::Drawable(ProjectionType projType,
                   const std::shared_ptr<Mesh>& mesh,
                   const std::shared_ptr<Texture>& texture,
                   const std::shared_ptr<Shader>& shader):
    mMesh(mesh),
    mTextures({ { "tex", texture ? texture : gResourceMgr.getDefaultTexture() } }),
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

void Drawable::setTexture(const std::string& uniformName,
                          const std::shared_ptr<const Texture>& tex)
{
    mTextures[uniformName] = tex;
}

void Drawable::setTexture(const std::shared_ptr<const Texture>& tex)
{
    setTexture("tex", tex);
}

namespace {

void setLightUniforms(const Renderer::State& state,
                      const std::shared_ptr<Shader>& shader)
{
    if (shader->hasUniform("eyePos")) {
        shader->setUniform("eyePos", state.camera->getEye());
    }

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

void setShadowUniforms(Renderer::State& state,
                       const std::shared_ptr<const Shader>& shader,
                       size_t firstTextureUnit,
                       std::vector<bind_guard<Texture>>& outBinds)
{
    sbAssert(shader->hasUniform("shadows") == shader->hasUniform("numShadows"),
             "shadows and numShadows must both be present");

    if (shader->hasUniform("shadows")) {
        shader->setUniform("numShadows", (unsigned)state.shadows.size());

        outBinds.clear();
        outBinds.reserve(state.shadows.size());

        for (size_t i = 0; i < state.shadows.size(); ++i) {
            std::string base = utils::format("shadows[{0}]", i);
            const Renderer::Shadow& s = state.shadows[i];

            outBinds.emplace_back(*s.shadowMap, firstTextureUnit + i);
            shader->setUniform(base + ".projectionMatrix", s.projectionMatrix);
            shader->setUniform(base + ".map", (GLint)(firstTextureUnit + i));
        }
    }
}

} // namespace

void Drawable::draw(Renderer::State& state) const
{
    if (state.isRenderingShadow
            && mProjectionType == ProjectionType::Orthographic) {
        return;
    }

    auto vaoBind = make_bind(mMesh->getVertexBuffer());
    auto indexBind = make_bind(mMesh->getIndexBuffer());
    auto shaderBind = make_bind(*mShader, mMesh->getVertexBuffer());

    mShader->setUniform("matViewProjection",
                        state.camera->getViewProjectionMatrix());
    mShader->setUniform("matModel", getTransformationMatrix());

    std::vector<bind_guard<Texture>> textureBinds;
    std::vector<bind_guard<Texture>> shadowBinds;
    if (!state.isRenderingShadow) {
        mShader->setUniform("color", mColor);

        size_t boundTextures = 0;
        for (const auto& pair: mTextures) {
            if (mShader->hasUniform(pair.first)) {
                textureBinds.emplace_back(make_bind(*pair.second, boundTextures));
                mShader->setUniform(pair.first, (GLint)boundTextures);
                ++boundTextures;
            }
        }

        setLightUniforms(state, mShader);
        setShadowUniforms(state, mShader, boundTextures, shadowBinds);
    }

    GL_CHECK(glDrawElements((GLuint)mMesh->getShape(),
                            mMesh->getIndexBufferSize(),
                            GL_UNSIGNED_INT, (void*)NULL));
}

}

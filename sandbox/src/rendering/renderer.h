#ifndef RENDERER_H
#define RENDERER_H

#include "rendering/includeGL.h"
#include <X11/Xlib.h>

#include "rendering/color.h"
#include "rendering/shader.h"
#include "rendering/texture.h"
#include "rendering/camera.h"
#include "rendering/light.h"
#include "rendering/framebuffer.h"

#include "utils/rect.h"

#include <vector>
#include <algorithm>

namespace sb
{
    class Drawable;

    class Renderer
    {
    private:
        struct UniformValue
        {
            enum class Type {
                Int,
                UInt,
                Float,
                Vec2,
                Vec3,
                Vec4,
                Mat3,
                Mat4
            } type;

            union ValueUnion {
                int32_t asInt;
                uint32_t asUInt;
                float asFloat;
                Vec2 asVec2;
                Vec3 asVec3;
                Vec4 asVec4;
                Mat33 asMat3;
                Mat44 asMat4;

                ValueUnion() {}
            } value;

            UniformValue(): UniformValue((int32_t)0) {}

            UniformValue(const UniformValue& copy) { *this = copy; }
            UniformValue& operator =(const UniformValue& copy)
            {
                type = copy.type;
                switch (type) {
                case Type::Int: value.asInt = copy.value.asInt; break;
                case Type::UInt: value.asUInt = copy.value.asUInt; break;
                case Type::Float: value.asFloat = copy.value.asFloat; break;
                case Type::Vec2: value.asVec2 = copy.value.asVec2; break;
                case Type::Vec3: value.asVec3 = copy.value.asVec3; break;
                case Type::Vec4: value.asVec4 = copy.value.asVec4; break;
                case Type::Mat3: value.asMat3 = copy.value.asMat3; break;
                case Type::Mat4: value.asMat4 = copy.value.asMat4; break;
                default: sbFail("invalid type");
                }

                return *this;
            }

            template<typename T>
            UniformValue(const T& val)
            {
                sbFail("invalid uniform type");
            }

            UniformValue(int32_t v):      type(Type::Int)   { value.asInt = v; }
            UniformValue(uint32_t v):     type(Type::UInt)  { value.asUInt = v; }
            UniformValue(float v):        type(Type::Float) { value.asFloat = v; }
            UniformValue(const Vec2& v):  type(Type::Vec2)  { value.asVec2 = v; }
            UniformValue(const Vec3& v):  type(Type::Vec3)  { value.asVec3 = v; }
            UniformValue(const Vec4& v):  type(Type::Vec4)  { value.asVec4 = v; }
            UniformValue(const Mat33& v): type(Type::Mat3)  { value.asMat3 = v; }
            UniformValue(const Mat44& v): type(Type::Mat4)  { value.asMat4 = v; }

            bool set(const std::string& name,
                     const Shader& shader) const
            {
                switch (type) {
                case Type::Int:   return shader.setUniform(name, value.asInt);
                case Type::UInt:  return shader.setUniform(name, value.asUInt);
                case Type::Float: return shader.setUniform(name, value.asFloat);
                case Type::Vec2:  return shader.setUniform(name, value.asVec2);
                case Type::Vec3:  return shader.setUniform(name, value.asVec3);
                case Type::Vec4:  return shader.setUniform(name, value.asVec4);
                case Type::Mat3:  return shader.setUniform(name, value.asMat3);
                case Type::Mat4:  return shader.setUniform(name, value.asMat4);
                }

                sbFail("invalid type");
                return false;
            }
        };

    public:
        struct Shadow
        {
            std::shared_ptr<const Texture> shadowMap;
            Mat44 projectionMatrix;
        };

        struct State
        {
            std::shared_ptr<Shader> shader;
            std::shared_ptr<Texture> texture;
            ProjectionType projectionType;
            Camera* camera;

            Color ambientLightColor;
            std::vector<Light> pointLights;
            std::vector<Light> parallelLights;
            std::vector<Shadow> shadows;
            const std::map<std::string, UniformValue>& uniforms;

            bool isRenderingShadow;
            ProjectionType shadowProjectionType;

        private:
            State(Camera& camera,
                  const Color& ambientLightColor,
                  const std::vector<Light>& allLights,
                  const std::map<std::string, UniformValue>& uniforms):
                shader(),
                texture(),
                projectionType(ProjectionType::Perspective),
                camera(&camera),
                ambientLightColor(ambientLightColor),
                uniforms(uniforms),
                isRenderingShadow(false)
            {
                std::copy_if(allLights.begin(), allLights.end(),
                             std::back_inserter(pointLights),
                             [](const Light& l) {
                                 return l.type == Light::Type::Point;
                             });
                std::copy_if(allLights.begin(), allLights.end(),
                             std::back_inserter(parallelLights),
                             [](const Light& l) {
                                 return l.type == Light::Type::Parallel;
                             });
            }

            friend class Renderer;
        };

        Renderer();
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator =(const Renderer&) = delete;
        Renderer& operator =(Renderer&&) = delete;

        inline Camera& getCamera() { return mCamera; }

        bool init(::Display* display, ::Window window, GLXFBConfig& fbc);
        void setClearColor(const Color& c);
        void clear() const;
        void setViewport(unsigned x, unsigned y, unsigned cx, unsigned cy);
        void setViewport(const IntRect& rect);

        void setAmbientLightColor(const Color& color) { mAmbientLightColor = color; }
        void addLight(const Light& light) { mLights.push_back(light); }

        void draw(Drawable& d);
        void draw(const std::shared_ptr<Drawable>& d) { draw(*d); }
        void drawAll();

        enum EFeature {
            FeatureBackfaceCulling = RENDERER_BACKFACE_CULLING,
            FeatureDepthTest = RENDERER_DEPTH_TEST,
            FeatureAlphaBlending = RENDERER_ALPHA_BLENDING
        };

        void enableFeature(EFeature feature, bool enable = true);
        void saveScreenshot(const std::string& filename, int width, int height);

        template<typename T>
        void setUniform(const std::string& name,
                        const T& value)
        {
            mUserUniforms[name] = UniformValue(value);
        }

    private:
        Color mClearColor;
        IntRect mViewport;
        Camera mCamera;
        Camera mSpriteCamera;
        GLXContext mGLContext;
        ::Display* mDisplay;

        std::vector<std::shared_ptr<Drawable>> mDrawablesBuffer;
        Color mAmbientLightColor;
        std::vector<Light> mLights;
        std::map<std::string, UniformValue> mUserUniforms;

        bool initGLEW();

        enum EFilterType {
            FilterShader,
            FilterTexture,
            FilterDepth,
            FilterProjection,
            FilterShaderTextureProjectionDepth
        };

        void drawTo(Framebuffer& framebuffer,
                    Camera& camera) const;
    };
} // namespace sb

#endif //RENDERER_H

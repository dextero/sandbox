#ifndef RENDERER_H
#define RENDERER_H

#include <sandbox/rendering/includeGL.h>

#include <sandbox/rendering/color.h>
#include <sandbox/rendering/shader.h>
#include <sandbox/rendering/texture.h>
#include <sandbox/rendering/camera.h>
#include <sandbox/rendering/light.h>
#include <sandbox/rendering/framebuffer.h>

#include <sandbox/utils/rect.h>

#include <X11/Xlib.h>

#include <vector>
#include <algorithm>

namespace sb
{
    class Drawable;

    class Renderer
    {
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

            bool isRenderingShadow;
            ProjectionType shadowProjectionType;

        private:
            State(Camera& camera,
                  const Color& ambientLightColor,
                  const std::vector<Light>& allLights):
                shader(),
                texture(),
                projectionType(ProjectionType::Perspective),
                camera(&camera),
                ambientLightColor(ambientLightColor),
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

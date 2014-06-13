#ifndef RENDERER_H
#define RENDERER_H

#include <cassert>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <X11/Xlib.h>

#include "rendering/color.h"
#include "rendering/shader.h"
#include "rendering/texture.h"
#include "rendering/camera.h"

#include <vector>

namespace sb
{
    class Drawable;

    class Renderer
    {
    public:
        struct State
        {
            std::shared_ptr<Shader> shader;
            std::shared_ptr<Texture> texture;
            EProjectionType projectionType;
            Camera &camera;

        private:
            State(Camera& camera):
                shader(),
                texture(),
                projectionType(EProjectionType::ProjectionPerspective),
                camera(camera)
            {}

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
        void clear();
        void setViewport(unsigned x, unsigned y, unsigned cx, unsigned cy);

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
        Camera mCamera;
        GLXContext mGLContext;
        ::Display* mDisplay;

        std::vector<std::shared_ptr<Drawable>> mDrawablesBuffer;

        bool initGLEW();

        enum EFilterType {
            FilterShader,
            FilterTexture,
            FilterDepth,
            FilterProjection,
            FilterShaderTextureProjectionDepth
        };
    };
} // namespace sb

#endif //RENDERER_H

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
#include "rendering/camera.h"
#include "rendering/drawable.h"

#include <vector>

namespace sb
{
    class Renderer
    {
    private:
        GLXContext mGLContext;
        ::Display* mDisplay;
        std::vector<Drawable> mDrawablesBuffer;
        bool mUseDrawableBuffering;

        bool initGLEW();

        enum EFilterType {
            FilterShader,
            FilterTexture,
            FilterDepth,
            FilterProjection,
            FilterShaderTextureProjectionDepth
        };
        void filterDrawables(EFilterType filter);

    public:
        Camera mCamera;

        Renderer();
        ~Renderer();

        bool init(::Display* display, ::Window window, GLXFBConfig& fbc);
        void setClearColor(const Color& c);
        void clear();
        void setViewport(unsigned x, unsigned y, unsigned cx, unsigned cy);

        void draw(Drawable& d);
        void drawAll();

        enum EFeature {
            FeatureBackfaceCulling = RENDERER_BACKFACE_CULLING,
            FeatureDepthTest = RENDERER_DEPTH_TEST,
            FeatureAlphaBlending = RENDERER_ALPHA_BLENDING
        };

        void enableFeature(EFeature feature, bool enable = true);
        void saveScreenshot(const std::string& filename, int width, int height);
    };
} // namespace sb

#endif //RENDERER_H

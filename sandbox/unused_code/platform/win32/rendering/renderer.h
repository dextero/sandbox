#ifndef RENDERER_H
#define RENDERER_H

#ifdef PLATFORM_WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#   undef near
#   undef far
#endif // PLATFORM_WIN32

#include <cassert>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

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
        HGLRC mGLContext;
        std::vector<Drawable> mDrawablesBuffer;
        bool mUseDrawableBuffering;

        bool InitGLEW();

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

        bool init(HWND wnd);
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

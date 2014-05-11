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

        bool InitGLEW();

        enum EFilterType {
            FilterShader,
            FilterTexture,
            FilterDepth,
            FilterProjection,
            FilterShaderTextureProjectionDepth
        };
        void FilterDrawables(EFilterType filter);

    public:
        Camera mCamera;

        Renderer();
        ~Renderer();

        bool Init(::Display* display, ::Window window, GLXFBConfig& fbc);
        void SetClearColor(const Color& c);
        void Clear();
        void SetViewport(unsigned x, unsigned y, unsigned cx, unsigned cy);

        void Draw(Drawable& d);
        void DrawAll();

        enum EFeature {
            FeatureBackfaceCulling = RENDERER_BACKFACE_CULLING,
            FeatureDepthTest = RENDERER_DEPTH_TEST,
            FeatureAlphaBlending = RENDERER_ALPHA_BLENDING
        };

        void EnableFeature(EFeature feature, bool enable = true);
        void SaveScreenshot(const wchar_t* filename, int width, int height);
    };
} // namespace sb

#endif //RENDERER_H

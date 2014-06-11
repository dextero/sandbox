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
                camera(camera)
            {}

            friend class Renderer;
        };

        Renderer();
        ~Renderer();

        inline Camera& getCamera() { return mCamera; }

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

    private:
        Camera mCamera;
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

        class DrawableComparator
        {
        public:
            typedef std::function<int(const Drawable&, const Drawable&)> Compare;

            template<typename T>
            static DrawableComparator compareBy(T comparator)
            {
                return DrawableComparator().thenBy(comparator);
            }

            DrawableComparator& thenBy(const Compare& comparator)
            {
                mChain.push_back(comparator);
                return *this;
            }

            template<typename T>
            DrawableComparator& thenBy(T Drawable::*field)
            {
                return thenBy([field](const Drawable& a, const Drawable& b) {
                    return a.*field - b.*field;
                });
            }

            template<typename T>
            DrawableComparator& thenBy(const std::shared_ptr<T> Drawable::*ptr)
            {
                return thenBy([ptr](const Drawable& a, const Drawable& b) {
                    return a.*ptr.get() - b.*ptr.get();
                });
            }

            operator Compare() const
            {
                assert(mChain.size() > 0);

                return [this]() {
                    std::vector<Compare> chain = mChain;

                    return [chain](const Drawable& a, const Drawable& b) {
                        for (auto it = chain.begin(); it != chain.end(); ++it) {
                            int value = (*it)(a, b);
                            if (value) {
                                return value;
                            }
                        }

                        return 0;
                    };
                }();
            }

        private:
            DrawableComparator() {}

            std::vector<Compare> mChain;
        };
    };
} // namespace sb

#endif //RENDERER_H

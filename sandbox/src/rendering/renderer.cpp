#include <algorithm>

#include "rendering/renderer.h"
#include "rendering/drawable.h"
#include "rendering/string.h"
#include "rendering/sprite.h"
#include "utils/lib.h"
#include "utils/stringUtils.h"
#include "utils/logger.h"
#include "utils/stl.h"
#include "utils/debug.h"
#include "resources/mesh.h"
#include "resources/image.h"
#include "rendering/model.h"

namespace sb {
namespace {

void printGLVersion() {
    int versionMajor = 0;
    int versionMinor = 0;
    GL_CHECK(glGetIntegerv(GL_MAJOR_VERSION, &versionMajor));
    GL_CHECK(glGetIntegerv(GL_MINOR_VERSION, &versionMinor));

    gLog.info("created GL %d.%d context", versionMajor, versionMinor);
    if (versionMajor != 0) {
        return;
    }

    gLog.warn("cannot get GL major/minor version, trying GL_VERSION string");
    const char* versionString = nullptr;
    GL_CHECK(versionString = (const char*)glGetString(GL_VERSION));
    if (!versionString) {
        sbFail("something went terribly wrong, cannot determine GL context version");
    }

    gLog.info("created GL context: %s", versionString);
}

} // namespace

bool Renderer::initGLEW()
{
    gLog.info("initializing GLEW...\n");
    glewExperimental = GL_TRUE;
    GLenum error = glewInit();

    // ignore any errors from inside GLEW
    glGetError();

    if (error != GLEW_OK) {
        gLog.err("glewInit failed: %s\n", glewGetErrorString(error));
        return false;
    } else {
        gLog.info("using GLEW %s\n", glewGetString(GLEW_VERSION));
    }

    utils::checkGLAvailability();
    return true;
}

Renderer::Renderer():
    mClearColor(Color::Black),
    mCamera(Camera::perspective()),
    mSpriteCamera(Camera::orthographic()),
    mGLContext(NULL),
    mDisplay(NULL),
    mDrawablesBuffer(),
    mAmbientLightColor(Color::White)
{
}

Renderer::~Renderer()
{
    // let's free everything before deleting gl context
    gResourceMgr.freeAll();

    glXMakeCurrent(mDisplay, 0, 0);
    if (mGLContext)
    {
        glXDestroyContext(mDisplay, mGLContext);
        mGLContext = NULL;

        gLog.info("GL context deleted\n");
    }
}

// typedef required for Renderer::init
typedef GLXContext (*GLXCREATECTXATTRSARBPROC)(::Display*, GLXFBConfig, GLXContext, Bool, const int*);

bool Renderer::init(::Display* display, ::Window window, GLXFBConfig& fbc)
{
    sbAssert(display, "display is null");

    mDisplay = display;

    gLog.info("creating GL context...\n");
    int contextAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        0
    };

    GLXCREATECTXATTRSARBPROC glXCreateContextAttribsARB = (GLXCREATECTXATTRSARBPROC)glXGetProcAddress((GLubyte*)"glXCreateContextAttribsARB");
    if (!glXCreateContextAttribsARB) {
        gLog.err("glXCreateContextAttribsARB not present\n");
        return false;
    }

    GL_CHECK(mGLContext = glXCreateContextAttribsARB(mDisplay, fbc, 0, True, contextAttribs));
    if (!mGLContext) {
        gLog.err("glXCreateContextAttribsARB failed\n");
        return false;
    }

    GL_CHECK(glXMakeCurrent(mDisplay, window, mGLContext));
    printGLVersion();

    if (!initGLEW()) {
        return false;
    }

    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glDepthFunc(GL_LESS));

    GL_CHECK(glEnable(GL_CULL_FACE));
    GL_CHECK(glCullFace(GL_BACK));

    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

#if 0
    GL_CHECK(glEnable(GL_TEXTURE_2D));

    String::init(mDisplay);
#endif

    return true;
}

void Renderer::setClearColor(const Color& c)
{
    mClearColor = c;
}

void Renderer::clear() const
{
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::setViewport(unsigned x, unsigned y, unsigned cx, unsigned cy)
{
    //gLog.debug("setViewport: %u %u %u %u", x, y, cx, cy);
    mViewport = IntRect(x, x + cx, y, y + cy);
    glViewport(x, y, cx, cy);

    // adjust aspect ratio
    mCamera.updateViewport(cx, cy);
    mSpriteCamera.updateViewport(cx, cy);
}

void Renderer::setViewport(const IntRect& rect)
{
    return setViewport(rect.left, rect.bottom, rect.width(), rect.height());
}

void Renderer::draw(Drawable& d)
{
    if (!d.mMesh && !d.mTexture) {
        sbFail("Renderer::draw: invalid call, mMesh == NULL");
    }

    mDrawablesBuffer.push_back(std::make_shared<Drawable>(d));
}

void Renderer::drawTo(Framebuffer& framebuffer,
                      Camera& camera) const
{
    auto fbBind = make_bind(framebuffer);
    GL_CHECK(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
    clear();

    //GL_CHECK(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE)); 

    State rendererState(camera, Color::White, {});
    rendererState.isRenderingShadow = true;
    rendererState.projectionType = ProjectionType::Orthographic; // TODO

    static Model ball("sphere.obj", gResourceMgr.getShader("proj_basic.vert", "color.frag"));
    ball.draw(rendererState);

    //for (const std::shared_ptr<Drawable>& d: mDrawablesBuffer) {
        //d->draw(rendererState);
    //}

    //GL_CHECK(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
}

void Renderer::drawAll()
{
    if (mDrawablesBuffer.size() == 0) {
        return;
    }

#if 0
    std::sort(mDrawablesBuffer.begin(), mDrawablesBuffer.end(),
              [](const std::shared_ptr<Drawable>& a,
                 const std::shared_ptr<Drawable>& b) {
                  return *a < *b;
              });
#endif
    State rendererState(mCamera,
                        mAmbientLightColor,
                        mLights);

    for (const Light& light: mLights) {
        if (light.makesShadows) {
            sbAssert(light.type == Light::Type::Parallel, "TODO: shadows for point lights");

            Camera camera = Camera::orthographic(-4.0, 4.0, -3.0, 3.0, -100.0, 100.0);
            //Camera camera = Camera::orthographic(-20.0, 20.0, -20.0, 20.0, -1000.0, 1000.0);
            //camera.lookAt(-light.pos + mCamera.getEye(), mCamera.getEye());

            IntRect savedViewport = mViewport;
            Vec2i shadowFbSize = light.shadowFramebuffer->getSize();
            setViewport(0, 0, shadowFbSize.x, shadowFbSize.y);

            drawTo(*light.shadowFramebuffer, camera);
            setViewport(savedViewport);

            rendererState.shadows.push_back({
                light.shadowFramebuffer->getTexture(),
                camera.getViewProjectionMatrix()
            });
        }
    }

    //sbFail("bo tak");

#define FUCKING_FBOS_HOW_DO_THEY_WORK 0
#if FUCKING_FBOS_HOW_DO_THEY_WORK
    {
        GL_CHECK(glClearColor(mClearColor.r, mClearColor.g,
                              mClearColor.b, mClearColor.a));
        clear();

        Camera camera = Camera::orthographic(-4.0, 4.0, -3.0, 3.0, -100.0, 100.0);
        camera.lookAt(-mLights[1].pos, Vec3());
        rendererState.camera = &camera;

        static Model ball("sphere.obj", gResourceMgr.getShader("proj_basic.vert", "color.frag"));
        ball.draw(rendererState);
    }
#else
    GL_CHECK(glClearColor(mClearColor.r, mClearColor.g,
                          mClearColor.b, mClearColor.a));
    clear();
    for (const std::shared_ptr<Drawable>& d: mDrawablesBuffer) {
        if (d->mProjectionType == ProjectionType::Perspective) {
            rendererState.camera = &mCamera;
        } else {
            rendererState.camera = &mSpriteCamera;
        }

        d->draw(rendererState);
    }
#endif

    mAmbientLightColor = Color::White;
    mLights.clear();
    mDrawablesBuffer.clear();
}

void Renderer::enableFeature(EFeature feature, bool enable)
{
    if (enable) {
        GL_CHECK(glEnable((GLenum)feature));
    } else {
        GL_CHECK(glDisable((GLenum)feature));
    }
}

void Renderer::saveScreenshot(const std::string& filename, int width, int height)
{
    ILuint tex;
    tex = IL_CHECK(ilGenImage());
    IL_CHECK(ilBindImage(tex));

    void* data = malloc(width * height * 3);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    ilTexImage(width, height, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, data);
    free(data);

    IL_CHECK(ilSaveImage(filename.c_str()));
    IL_CHECK(ilDeleteImage(tex));
}

} // namespace sb


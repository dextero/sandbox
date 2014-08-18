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
    mCamera(),
    mGLContext(NULL),
    mDisplay(NULL),
    mDrawablesBuffer()
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

#if 0
    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    GL_CHECK(glEnable(GL_TEXTURE_2D));

    String::init(mDisplay);
#endif

    mCamera.setPerspectiveMatrix();
    mCamera.setOrthographicMatrix();

    return true;
}

void Renderer::setClearColor(const Color& c)
{
    glClearColor(c.r, c.g, c.b, c.a);
}

void Renderer::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setViewport(unsigned x, unsigned y, unsigned cx, unsigned cy)
{
    glViewport(x, y, cx, cy);

    // adjust aspect ratio
    mCamera.setPerspectiveMatrix(PI_3, (float)cx / (float)cy);
}

void Renderer::draw(Drawable& d)
{
    if (!d.mMesh && !d.mTexture)
    {
        gLog.err("Renderer::draw: invalid call, mMesh == NULL\n");
        sbFail("Renderer::draw: invalid call");
        return;
    }

    mDrawablesBuffer.push_back(std::make_shared<Drawable>(d));
}

void Renderer::drawAll()
{
    if (mDrawablesBuffer.size() == 0) {
        return;
    }

    std::sort(mDrawablesBuffer.begin(), mDrawablesBuffer.end(),
              [](const std::shared_ptr<Drawable>& a,
                 const std::shared_ptr<Drawable>& b) {
                  return *a < *b;
              });

    State rendererState(mCamera);
    for (const std::shared_ptr<Drawable>& d: mDrawablesBuffer) {
        d->draw(rendererState);
    }

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


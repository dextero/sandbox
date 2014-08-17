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


namespace sb
{
    bool Renderer::initGLEW()
    {
        gLog.info("initializing GLEW...\n");
        GLenum error = glewInit();
        if (error != GLEW_OK)
        {
            gLog.err("glewInit failed: %s\n", glewGetErrorString(error));
            return false;
        }
        else
            gLog.info("using GLEW %s\n", glewGetString(GLEW_VERSION));

        gLog.info("checking GL functions availability\n");
        static struct GLFunc {
            const void* func;
            const char* name;
            const char* errMsg;
            enum {
                SevOptional,
                SevRequired
            } severity;
        } functions[] = {
#define FUNC_OPT(name, fail_msg) { (const void*)(name), #name, (fail_msg), GLFunc::SevOptional }
#define FUNC_REQ(name, fail_msg) { (const void*)(name), #name, (fail_msg), GLFunc::SevRequired }
            FUNC_REQ(glGenVertexArrays, 0),
            FUNC_REQ(glDeleteVertexArrays, 0),
            FUNC_REQ(glBindVertexArray, 0),
            FUNC_REQ(glGetVertexAttribiv, 0),
            FUNC_REQ(glVertexAttribPointer, 0),
            FUNC_REQ(glBindAttribLocation, 0),
            FUNC_OPT(glEnableVertexAttribArray, 0),
            FUNC_OPT(glDisableVertexAttribArray, 0),
            FUNC_REQ(glGenBuffers, 0),
            FUNC_REQ(glDeleteBuffers, 0),
            FUNC_REQ(glBindBuffer, 0),
            FUNC_OPT(glCopyBufferSubData, "in-GPU copying no available, will use RAM\n"), // if present, GL_COPY_READ_BUFFER & GL_COPY_WRITE_BUFFER should be available too
            FUNC_REQ(glBufferSubData, 0),
            FUNC_REQ(glGetBufferParameteriv, 0),
            FUNC_REQ(glActiveTexture, 0),
            FUNC_OPT(glGenerateMipmap, "will use glGenerateMipmapEXT if available\n"),
            FUNC_OPT(glGenerateMipmapEXT, "bye bye mipmaps :(\n"),
            FUNC_REQ(glDrawElements, 0),
            FUNC_REQ(glUseProgram, 0),
            FUNC_REQ(glCreateProgram, 0),
            FUNC_REQ(glLinkProgram, 0),
            FUNC_REQ(glDeleteProgram, 0),
            FUNC_REQ(glCreateShader, 0),
            FUNC_REQ(glShaderSource, 0),
            FUNC_REQ(glCompileShader, 0),
            FUNC_REQ(glAttachShader, 0),
            FUNC_REQ(glDeleteShader, 0),
            FUNC_REQ(glDrawElements, 0),
            FUNC_REQ(glUniform1iv, 0),
            FUNC_REQ(glUniform1fv, 0),
            FUNC_REQ(glUniform2fv, 0),
            FUNC_REQ(glUniform3fv, 0),
            FUNC_REQ(glUniform4fv, 0),
            FUNC_REQ(glUniformMatrix4fv, 0)
#undef FUNC_OPT
#undef FUNC_REQ
        };

        uint32_t requiredFunctionsMissing = 0;
        for (size_t i = 0; i < sizeof(functions) / sizeof(functions[0]); ++i)
        {
            if (functions[i].func != NULL)
            {
                gLog.info("%-40sOK\n", functions[i].name);
            }
            else
            {
                if (functions[i].severity == GLFunc::SevOptional)
                {
                    gLog.warn("%-40sNOT AVAILABLE\n", functions[i].name);
                }
                else
                {
                    gLog.err("%-40sNOT AVAILABLE\n", functions[i].name);
                    ++requiredFunctionsMissing;
                }

                if (functions[i].errMsg != NULL)
                {
                    gLog.info(functions[i].errMsg);
                }
            }
        }

        if (requiredFunctionsMissing > 0)
            gLog.err("some critical GL functions missing, app will most likely crash\n");

        return requiredFunctionsMissing == 0;
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
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            None
        };

        GLXCREATECTXATTRSARBPROC glXCreateContextAttribsARB = (GLXCREATECTXATTRSARBPROC)glXGetProcAddress((GLubyte*)"glXCreateContextAttribsARB");
        if (!glXCreateContextAttribsARB)
        {
            gLog.err("glXCreateContextAttribsARB not present\n");
            return false;
        }

        mGLContext = glXCreateContextAttribsARB(mDisplay, fbc, 0, True, contextAttribs);
        if (!mGLContext)
        {
            gLog.err("glXCreateContextAttribsARB failed\n");
            return false;
        }

        glXMakeCurrent(mDisplay, window, mGLContext);

        if (!initGLEW())
            return false;

        GL_CHECK(glEnable(GL_DEPTH_TEST));
        GL_CHECK(glDepthFunc(GL_LESS));

        GL_CHECK(glEnable(GL_CULL_FACE));
        GL_CHECK(glCullFace(GL_BACK));

        GL_CHECK(glEnable(GL_BLEND));
        GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        GL_CHECK(glEnable(GL_TEXTURE_2D));

        String::init(mDisplay);

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


#include <algorithm>

#include "rendering/renderer.h"
#include "rendering/drawable.h"
#include "rendering/string.h"
#include "rendering/sprite.h"
#include "utils/libUtils.h"
#include "utils/stringUtils.h"
#include "utils/profiler.h"
#include "utils/logger.h"
#include "resources/mesh.h"
#include "resources/image.h"


namespace sb
{
    bool Renderer::InitGLEW()
    {
        PROFILE();

        gLog.Info("initializing GLEW...\n");
        GLenum error = glewInit();
        if (error != GLEW_OK)
        {
            gLog.Err("glewInit failed: %s\n", glewGetErrorString(error));
            return false;
        }
        else
            gLog.Info("using GLEW %s\n", glewGetString(GLEW_VERSION));

        gLog.Info("checking GL functions availability\n");
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
                gLog.Info("%s \tOK\n", functions[i].name);
            else
            {
                if (functions[i].severity == GLFunc::SevOptional)
                    gLog.Warn("%s \tNOT AVAILABLE\n", functions[i].name);
                else
                {
                    gLog.Err("%s \tNOT AVAILABLE\n", functions[i].name);
                    ++requiredFunctionsMissing;
                }

                if (functions[i].errMsg != NULL)
                    gLog.Info(functions[i].errMsg);
            }
        }

        if (requiredFunctionsMissing > 0)
            gLog.Err("some critical GL functions missing, app will most likely crash\n");

        return requiredFunctionsMissing == 0;
    }

    void Renderer::FilterDrawables(EFilterType filter)
    {
        switch(filter)
        {
        case FilterShader:
            std::sort(mDrawablesBuffer.begin(), mDrawablesBuffer.end(), [](const Drawable& first, const Drawable& second) -> bool {
                return first.GetShader() < second.GetShader();
            });
            break;
        case FilterTexture:
            std::sort(mDrawablesBuffer.begin(), mDrawablesBuffer.end(), [](const Drawable& first, const Drawable& second) -> bool {
                return first.mTexture < second.mTexture;
            });
            break;
        case FilterDepth:
            std::sort(mDrawablesBuffer.begin(), mDrawablesBuffer.end(), [](const Drawable& first, const Drawable& second) -> bool {
                Vec4 f(first.GetPosition()[0], first.GetPosition()[1], first.GetPosition()[2], 1.f),
                     s(second.GetPosition()[0], second.GetPosition()[1], second.GetPosition()[2], 1.f);
                f = const_cast<Drawable&>(first).GetTransformationMatrix() * f;
                s = const_cast<Drawable&>(second).GetTransformationMatrix() * f;
                return f[2] < s[2];
            });
            break;
        case FilterProjection:
            std::sort(mDrawablesBuffer.begin(), mDrawablesBuffer.end(), [](const Drawable& first, const Drawable& second) -> bool {
                return first.mProjectionType < second.mProjectionType;
            });
            break;
        case FilterShaderTextureProjectionDepth:
            std::sort(mDrawablesBuffer.begin(), mDrawablesBuffer.end(), [](const Drawable& first, const Drawable& second) -> bool {
                if (first.GetShader() == second.GetShader())
                {
                    if (first.mTexture == second.mTexture)
                    {
                        if (first.mProjectionType == second.mProjectionType)
                        {
                            Vec4 f(first.GetPosition()[0], first.GetPosition()[1], first.GetPosition()[2], 1.f),
                                 s(second.GetPosition()[0], second.GetPosition()[1], second.GetPosition()[2], 1.f);
                            f = const_cast<Drawable&>(first).GetTransformationMatrix() * f;
                            s = const_cast<Drawable&>(second).GetTransformationMatrix() * f;
                            return f[2] < s[2];
                        }
                        else
                            return first.mProjectionType < second.mProjectionType;
                    }
                    else
                        return first.mTexture < second.mTexture;
                }
                else
                    return first.GetShader() < second.GetShader();
            });
            break;
        }
    }


    Renderer::Renderer():
        mGLContext(NULL),
        mDisplay(NULL),
        mUseDrawableBuffering(false)
    {
    }

    Renderer::~Renderer()
    {
        PROFILE();

        // let's free everything before deleting gl context
        Shader::FreeShaders();
        gResourceMgr.FreeAllResources();

        glXMakeCurrent(mDisplay, 0, 0);
        if (mGLContext)
        {
            glXDestroyContext(mDisplay, mGLContext);
            mGLContext = NULL;

            gLog.Info("GL context deleted\n");
        }
    }

    // typedef required for Renderer::Init
    typedef GLXContext (*GLXCREATECTXATTRSARBPROC)(::Display*, GLXFBConfig, GLXContext, Bool, const int*);

    bool Renderer::Init(::Display* display, ::Window window, GLXFBConfig& fbc)
    {
        PROFILE();

        assert(display);

        mDisplay = display;

        gLog.Info("creating GL context...\n");
        int contextAttribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            None
        };

        GLXCREATECTXATTRSARBPROC glXCreateContextAttribsARB = (GLXCREATECTXATTRSARBPROC)glXGetProcAddress((GLubyte*)"glXCreateContextAttribsARB");
        if (!glXCreateContextAttribsARB)
        {
            gLog.Err("glXCreateContextAttribsARB not present\n");
            return false;
        }

        mGLContext = glXCreateContextAttribsARB(mDisplay, fbc, 0, True, contextAttribs);
        if (!mGLContext)
        {
            gLog.Err("glXCreateContextAttribsARB failed\n");
            return false;
        }

        glXMakeCurrent(mDisplay, window, mGLContext);

        if (!InitGLEW())
            return false;

        GL_CHECK(glEnable(GL_DEPTH_TEST));
        GL_CHECK(glDepthFunc(GL_LESS));

        GL_CHECK(glEnable(GL_CULL_FACE));
        GL_CHECK(glCullFace(GL_BACK));

        GL_CHECK(glEnable(GL_BLEND));
        GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        GL_CHECK(glEnable(GL_TEXTURE_2D));

        Shader::InitShaders();
        String::Init(mDisplay);

        mCamera.SetPerspectiveMatrix();

        mCamera.SetOrthographicMatrix();

        return true;
    }

    void Renderer::SetClearColor(const Color& c)
    {
        PROFILE();

        glClearColor(c.r, c.g, c.b, c.a);
    }

    void Renderer::Clear()
    {
        PROFILE();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::SetViewport(unsigned x, unsigned y, unsigned cx, unsigned cy)
    {
        PROFILE();

        glViewport(x, y, cx, cy);

        // adjust aspect ratio
        mCamera.SetPerspectiveMatrix(PI_3, (float)cx / (float)cy);
    }

    void Renderer::Draw(Drawable& d)
    {
        PROFILE();

        if (!d.mMesh && !d.mTexture)
        {
            gLog.Err("Renderer::Draw: invalid call, mMesh == NULL\n");
            assert(!"Renderer::Draw: invalid call");
            return;
        }

        if (mUseDrawableBuffering)
            mDrawablesBuffer.push_back(d);
        else
        {
            Mesh& mesh = *(d.mMesh ? d.mMesh : gResourceMgr.GetSprite(d.mTexture));
            TextureId texture = d.mTexture;
            if (!texture)
                texture = mesh.GetTexture();

            // should be before Shader::Use to ensure that glBindAttribLocation calls are correct
            mesh.GetVertexBuffer().Bind();

            if (texture)
            {
                Shader::Use(Shader::ShaderTexture);
                Shader::GetCurrent().SetUniform("u_texture", (int)Shader::SamplerImage);
                GL_CHECK(glActiveTexture(GL_TEXTURE0 + Shader::SamplerImage));
                GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
            }
            else
            {
                Shader::Use(Shader::ShaderColor);
            }

            Shader& shader = Shader::GetCurrent();
            shader.SetUniform("u_color", d.mColor);

            if (d.mProjectionType == Drawable::ProjectionOrthographic)
                shader.SetUniform("u_matViewProjection", mCamera.GetOrthographicProjectionMatrix());
            else
                shader.SetUniform("u_matViewProjection", Mat44(
                    mCamera.GetPerspectiveProjectionMatrix() * mCamera.GetViewMatrix()));

            shader.SetUniform("u_matModel", d.GetTransformationMatrix());

            GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.GetIndexBuffer()));
            GL_CHECK(glDrawElements(mesh.GetShape(), mesh.GetIndexBufferSize(), GL_UNSIGNED_INT, 0));

            mesh.GetVertexBuffer().Unbind();
        }
    }

    void Renderer::DrawAll()
    {
        if (mDrawablesBuffer.size() == 0)
            return;

        FilterDrawables(FilterShaderTextureProjectionDepth);

        Shader::EShader shaderType = mDrawablesBuffer[0].GetShader();
        Shader::Use(shaderType);

        Shader* shader = &Shader::GetCurrent();
        if (shaderType == Shader::ShaderTexture)
            shader->SetUniform("u_texture", (int)Shader::SamplerImage);

        TextureId texture = (mDrawablesBuffer[0].mTexture ? mDrawablesBuffer[0].mTexture : mDrawablesBuffer[0].mMesh->GetTexture());
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + Shader::SamplerImage));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));

        Drawable::EProjectionType projType = mDrawablesBuffer[0].mProjectionType;
        shader->SetUniform("u_matViewProjection", projType == Drawable::ProjectionOrthographic ?
            mCamera.GetOrthographicProjectionMatrix() :
            Mat44(mCamera.GetPerspectiveProjectionMatrix() * mCamera.GetViewMatrix()));

        Mesh::GetVertexBuffer().Bind();

        for (std::vector<Drawable>::iterator it = mDrawablesBuffer.begin(); it != mDrawablesBuffer.end(); ++it)
        {
            if (it->GetShader() != shaderType)
            {
                shaderType = it->GetShader();
                Shader::Use(shaderType);
                shader = &Shader::GetCurrent();

                if (shaderType == Shader::ShaderTexture)
                    shader->SetUniform("u_texture", Shader::SamplerImage);
                shader->SetUniform("u_matViewProjection", projType == Drawable::ProjectionOrthographic ?
                    mCamera.GetOrthographicProjectionMatrix() :
                    Mat44(mCamera.GetPerspectiveProjectionMatrix() * mCamera.GetViewMatrix()));
            }

            TextureId tex = (it->mTexture ? it->mTexture : it->mMesh->GetTexture());
            if (tex != texture)
            {
                texture = tex;
                GL_CHECK(glActiveTexture(GL_TEXTURE0 + Shader::SamplerImage));
                GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
            }

            if (it->mProjectionType != projType)
            {
                projType = it->mProjectionType;
                shader->SetUniform("u_matViewProjection", projType == Drawable::ProjectionOrthographic ?
                    mCamera.GetOrthographicProjectionMatrix() :
                    Mat44(mCamera.GetPerspectiveProjectionMatrix() * mCamera.GetViewMatrix()));
            }

            shader->SetUniform("u_matModel", it->GetTransformationMatrix());
            shader->SetUniform("u_color", it->mColor);

            Mesh* mesh = (it->mMesh ? it->mMesh : gResourceMgr.GetSprite(it->mTexture));
            GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIndexBuffer()));
            GL_CHECK(glDrawElements(mesh->GetShape(), mesh->GetIndexBufferSize(), GL_UNSIGNED_INT, NULL));
        }

        mDrawablesBuffer.clear();
        Mesh::GetVertexBuffer().Unbind();
    }

    void Renderer::EnableFeature(EFeature feature, bool enable)
    {
        PROFILE();

        if (enable)
            GL_CHECK(glEnable((GLenum)feature));
        else
            GL_CHECK(glDisable((GLenum)feature));
    }

    void Renderer::SaveScreenshot(const wchar_t* filename, int width, int height)
    {
        ILuint tex;
        tex = IL_CHECK(ilGenImage());
        IL_CHECK(ilBindImage(tex));

        void* data = malloc(width * height * 3);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
        ilTexImage(width, height, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, data);
        free(data);

        IL_CHECK(ilSaveImage(StringUtils::ToString(filename).c_str()));
        IL_CHECK(ilDeleteImage(tex));
    }
} // namespace sb


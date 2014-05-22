#include "shader.h"
#include "sharedVertexBuffer.h"
#include "../resources/resourceMgr.h"
#include "../utils/libUtils.h"
#include "../utils/stringUtils.h"
#include "../utils/logger.h"

#include <cstring>

namespace sb
{
    std::vector<Shader> Shader::msShaders = std::vector<Shader>();
    std::vector<std::vector<std::pair<uint32_t, std::string> > > Shader::msAttribs;
    Shader::EShader    Shader::msCurrent = Shader::ShaderCount;

    GLenum Shader::translateShaderType(EShaderType type)
    {
        switch (type)
        {
        case ShaderTypeVertex: return GL_VERTEX_SHADER;
        case ShaderTypeFragment: return GL_FRAGMENT_SHADER;
        case ShaderTypeGeometry: return GL_GEOMETRY_SHADER;
        default:
            assert(!"Invalid shader type!");
            gLog.err("Invalid shader type: %d. Assuming it's a vertex shader\n");
            return GL_VERTEX_SHADER;
        }
    }

    bool Shader::checkCompilationStatus(ShaderId shader)
    {
        GLint retval;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &retval);
        if (retval == GL_FALSE)
        {
            // compilation failed!
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &retval);

            gLog.err("Compilation failed! Log:\n");
            if (retval > 0)
            {
                char* buffer = new char[retval];

                glGetShaderInfoLog(shader, retval - 1, &retval, buffer);
                gLog.info("%s\n", buffer);

                delete[] buffer;
            }

            return false;
        }

        return true;
    }

    bool Shader::checkLinkStatus(ProgramId program)
    {
        GLint retval;
        glGetProgramiv(program, GL_LINK_STATUS, &retval);
        if (retval == GL_FALSE)
        {
            // link failed!
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &retval);

            gLog.err("Linking failed! Log:\n");
            if (retval > 0)
            {
                char* buffer = new char[retval];

                glGetProgramInfoLog(program, retval - 1, &retval, buffer);
                gLog.info("%s\n", buffer);

                delete[] buffer;
            }

            return false;
        }

        return true;
    }


    Shader::Shader(): mProgram(0)
    {
        for (int i = 0; i < ShaderTypesCount; ++i)
            mShaders[i] = 0;
    }

    Shader::~Shader()
    {
        free();
    }

    bool Shader::loadShader(EShaderType type, const char* file)
    {
        std::string path = gResourceMgr.getShaderPath() + file;

    #define _CRT_SECURE_NO_WARNINGS
        FILE* f = fopen(path.c_str(), "r");
    #undef _CRT_SECURE_NO_WARNINGS

        if (!f)
            return false;

        fseek(f, 0, SEEK_END);
        int filesize = ftell(f);
        fseek(f, 0, SEEK_SET);

        char* buffer = new char[filesize + 1];
        memset(buffer, 0, (filesize + 1) * sizeof(char));

        fread(buffer, 1, filesize, f);
        fclose(f);

        if (mShaders[type])
            GL_CHECK(glDeleteShader(mShaders[type]));

        GL_CHECK(mShaders[type] = glCreateShader(translateShaderType(type)));
        GL_CHECK(glShaderSource(mShaders[type], 1, (const GLchar**)&buffer, NULL));

        static const char* shaders[] = { "vertex", "fragment", "geometry" };
        gLog.info("compiling %s shader: %s...\n", shaders[type], file);
        GL_CHECK(glCompileShader(mShaders[type]));
        if (!checkCompilationStatus(mShaders[type]))
            return false;

        delete[] buffer;
        return true;
    }

    bool Shader::load(const char* vertex, const char* fragment, const char* geometry)
    {
        gLog.info("loading shader: %s, %s, %s\n", vertex, fragment, geometry ? geometry : "(no geometry shader)");

        bool ret = loadShader(ShaderTypeVertex, vertex);
        ret &= loadShader(ShaderTypeFragment, fragment);
        if (geometry) ret &= loadShader(ShaderTypeGeometry, geometry);

        if (ret) {
            ret = compileAndLink();
        }

        return ret;
    }

    bool Shader::compileAndLink()
    {
        if (mProgram)
            GL_CHECK(glDeleteProgram(mProgram));
        mProgram = GL_CHECK(glCreateProgram());

        for (int i = ShaderTypeVertex; i != ShaderTypesCount; ++i)
            if (mShaders[i])
                GL_CHECK(glAttachShader(mProgram, mShaders[i]));

        gLog.info("linking shader program...\n");
        GL_CHECK(glLinkProgram(mProgram));

        return checkLinkStatus(mProgram);
    }

    void Shader::free()
    {
        for (int i = ShaderTypeVertex; i != ShaderTypesCount; ++i)
            if (mShaders[i])
            {
                GL_CHECK(glDeleteShader(mShaders[i]));
                mShaders[i] = 0;
            }

        if (mProgram)
        {
            GL_CHECK(glDeleteProgram(mProgram));
            mProgram = 0;
        }
    }

    #define SET_UNIFORM(funccall) \
        if (!mProgram) return false; \
        GLint loc = glGetUniformLocation(mProgram, name); \
        if (loc == -1) return false; \
        GL_CHECK(funccall); \
        return true

    bool Shader::setUniform(const char* name, const float* value_array, uint32_t elements)
    {
        SET_UNIFORM(glUniform1fv(loc, elements, (const GLfloat*)value_array));
    }

    bool Shader::setUniform(const char* name, const Vec2* value_array, uint32_t elements)
    {
        SET_UNIFORM(glUniform2fv(loc, elements, (const GLfloat*)value_array));
    }

    bool Shader::setUniform(const char* name, const Vec3* value_array, uint32_t elements)
    {
        SET_UNIFORM(glUniform3fv(loc, elements, (const GLfloat*)value_array));
    }

    bool Shader::setUniform(const char* name, const Color* value_array, uint32_t elements)
    {
        SET_UNIFORM(glUniform4fv(loc, elements, (const GLfloat*)value_array));
    }

    bool Shader::setUniform(const char* name, const Mat44* value_array, uint32_t elements)
    {
        SET_UNIFORM(glUniformMatrix4fv(loc, elements, GL_FALSE, (const GLfloat*)value_array));
    }

    bool Shader::setUniform(const char* name, const int* value_array, uint32_t elements)
    {
        SET_UNIFORM(glUniform1iv(loc, elements, (const GLint*)value_array));
    }

    void Shader::initShaders()
    {
        gLog.info("initializing shaders...\n");

        msShaders.resize(ShaderCount);
        msAttribs.resize(ShaderCount);

        msShaders[ShaderTexture].load("proj_texture.vert", "texture.frag");
        msShaders[ShaderColor].load("proj_color.vert", "color.frag");
        msShaders[ShaderPointSprite].load("proj_texture.vert", "color.frag", "point_sprite.geom");

        msAttribs[ShaderTexture].push_back(std::make_pair(SharedVertexBuffer::BufferVertex, "a_vertex"));
        msAttribs[ShaderTexture].push_back(std::make_pair(SharedVertexBuffer::BufferColor, "a_color"));
        msAttribs[ShaderTexture].push_back(std::make_pair(SharedVertexBuffer::BufferTexcoord, "a_texcoord"));

        msAttribs[ShaderColor].push_back(std::make_pair(SharedVertexBuffer::BufferVertex, "a_vertex"));
        msAttribs[ShaderColor].push_back(std::make_pair(SharedVertexBuffer::BufferColor, "a_color"));

        msAttribs[ShaderPointSprite].push_back(std::make_pair(SharedVertexBuffer::BufferVertex, "a_vertex"));
    }

    void Shader::freeShaders()
    {
        msShaders.clear();
        msAttribs.clear();

        gLog.info("all shaders freed\n");
    }

    Shader& Shader::get(EShader shader)
    {
        return msShaders[shader];
    }

    Shader& Shader::getCurrent()
    {
        return msShaders[msCurrent];
    }

    void Shader::use(EShader shader)
    {
        if (msCurrent < ShaderCount)
            for (std::vector<std::pair<uint32_t, std::string> >::iterator it = msAttribs[msCurrent].begin(); it != msAttribs[msCurrent].end(); ++it)
                GL_CHECK(glDisableVertexAttribArray(it->first));

        if (shader != msCurrent)
        {
            if (shader == ShaderNone)
                GL_CHECK(glUseProgram(0));
            else
                GL_CHECK(glUseProgram(msShaders[shader].getProgram()));
            msCurrent = shader;
        }

        if ((size_t)msCurrent < msAttribs.size())
            for (std::vector<std::pair<uint32_t, std::string> >::iterator it = msAttribs[msCurrent].begin(); it != msAttribs[msCurrent].end(); ++it)
            {
                GL_CHECK(glEnableVertexAttribArray(it->first));
                GL_CHECK(glBindAttribLocation(msShaders[msCurrent].getProgram(), it->first, it->second.c_str()));
            }
    }
} // namespace sb

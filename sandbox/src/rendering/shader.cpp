#include "rendering/shader.h"
#include "rendering/vertexBuffer.h"
#include "resources/resourceMgr.h"
#include "utils/lib.h"
#include "utils/stringUtils.h"
#include "utils/logger.h"

#include <cstring>

namespace sb
{
    Shader::Shader(const std::shared_ptr<ShaderId>& vertex,
                   const std::shared_ptr<ShaderId>& fragment,
                   const std::shared_ptr<ShaderId>& geometry,
                   const std::vector<std::string>& attribs):
        mProgram(linkShader(vertex, fragment, geometry)),
        mVertexShader(vertex),
        mFragmentShader(fragment),
        mGeometryShader(geometry),
        mAttribs(attribs)
    {}

    std::shared_ptr<ProgramId>
    Shader::linkShader(const std::shared_ptr<ShaderId>& vertex,
                       const std::shared_ptr<ShaderId>& fragment,
                       const std::shared_ptr<ShaderId>& geometry)
    {
        ProgramId id;
        GL_CHECK(id = glCreateProgram());
        if (!id) {
            return {};
        }

        if (vertex) {
            GL_CHECK(glAttachShader(id, *vertex));
        }
        if (fragment) {
            GL_CHECK(glAttachShader(id, *fragment));
        }
        if (geometry) {
            GL_CHECK(glAttachShader(id, *geometry));
        }

        gLog.trace("linking shader program...");
        GL_CHECK(glLinkProgram(id));

        if (!shaderLinkSucceeded(id)) {
            gLog.err("shader link failed");
            return {};
        }

        return std::make_shared<ProgramId>(id);
    }

    bool Shader::shaderLinkSucceeded(ProgramId program)
    {
        GLint retval;
        GL_CHECK_RET(glGetProgramiv(program, GL_LINK_STATUS, &retval), false);
        if (retval == GL_FALSE)
        {
            // link failed!
            GL_CHECK_RET(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &retval),
                         false);

            gLog.err("linking failed! log:");
            if (retval > 0)
            {
                std::string buffer;
                buffer.resize(retval);

                GL_CHECK_RET(glGetProgramInfoLog(program, retval - 1,
                                                 &retval, &buffer[0]), false);
                gLog.printf("%s", buffer.c_str());
            }

            return false;
        }

        return true;
    }

    #define DEFINE_UNIFORM_SETTER(Type, funccall) \
        bool Shader::setUniform(const char* name, const Type* value_array, uint32_t elements) \
        { \
            if (!mProgram) return false; \
            GLint loc = glGetUniformLocation(*mProgram, name); \
            if (loc == -1) return false; \
            GL_CHECK(funccall); \
            return true; \
        }

    DEFINE_UNIFORM_SETTER(float, glUniform1fv)
    DEFINE_UNIFORM_SETTER(Vec2, glUniform2fv)
    DEFINE_UNIFORM_SETTER(Vec3, glUniform3fv)
    DEFINE_UNIFORM_SETTER(Color, glUniform4fv)
    DEFINE_UNIFORM_SETTER(Mat44, glUniformMatrix4fv)
    DEFINE_UNIFORM_SETTER(int, glUniform1iv)

    void Shader::bind()
    {
        gLog.debug("bind program %d\n", *mProgram);
        GL_CHECK(glUseProgram(*mProgram));

        for (size_t i = 0; i < mAttribs.size(); ++i) {
            const std::string& name = mAttribs[i];
            gLog.debug("- enable attrib %lu (%s)\n", i, name.c_str());
            GL_CHECK(glEnableVertexAttribArray((GLuint)i));
            GL_CHECK(glBindAttribLocation(*mProgram, (GLuint)i, name.c_str()));
        }
    }

    void Shader::unbind()
    {
        gLog.debug("unbind program\n");
        GL_CHECK(glUseProgram(0));

        for (size_t i = 0; i < mAttribs.size(); ++i) {
            const std::string& name = mAttribs[i];
            gLog.debug("- disable attrib %lu (%s)\n", i, name.c_str());
            GL_CHECK(glDisableVertexAttribArray((GLuint)i));
        }
    }
} // namespace sb

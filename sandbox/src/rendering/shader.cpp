#include "rendering/shader.h"
#include "rendering/vertexBuffer.h"
#include "resources/resourceMgr.h"
#include "utils/lib.h"
#include "utils/stringUtils.h"
#include "utils/logger.h"

#include <cstring>
#include <algorithm>

namespace sb {
namespace {

void extractInput(std::map<Attrib::Kind, Input>& outInputs,
                  const std::string& line)
{
    std::vector<std::string> words = utils::split(line);

    if (line.size() == 0
            || words[0] != "in"
            || words.size() < 5) {
        return;
    }

    std::map<std::string, Attrib::Kind> ATTRIB_KINDS {
        { "POSITION", Attrib::Kind::Position },
        { "TEXCOORD", Attrib::Kind::Texcoord },
        { "COLOR", Attrib::Kind::Color },
        { "NORMAL", Attrib::Kind::Normal }
    };

    const std::string& type = words[1];
    const std::string& name = utils::strip(words[2], ";");
    const std::string& kind_str = words[4];

    auto kindIt = ATTRIB_KINDS.find(kind_str);
    if (kindIt == ATTRIB_KINDS.end()) {
        sbFail("unknown attribute kind: %s", kind_str.c_str());
    }
    const Attrib::Kind kind = kindIt->second;

    auto it = outInputs.find(kind);
    if (it != outInputs.end()) {
        sbFail("multiple inputs of the same kind (%s) detected: %s and %s",
               kind_str.c_str(), it->second.name.c_str(), name.c_str());
    }

    gLog.trace("input (%s): %s %s",
               kind_str.c_str(), type.c_str(), name.c_str());

    outInputs[kind] = { kind, type, name };
}

} // namespace

std::map<Attrib::Kind, Input>
ConcreteShader::getInputs(const std::string& code)
{
    std::map<Attrib::Kind, Input> ret;
    std::vector<std::string> lines = utils::split(code, "\n");

    for (const std::string& line: lines) {
        extractInput(ret, line);
    }

    return ret;
}

bool ConcreteShader::shaderCompilationSucceeded()
{
    GLint retval;
    GL_CHECK_RET(glGetShaderiv(mShader, GL_COMPILE_STATUS, &retval), false);
    if (retval == GL_FALSE)
    {
        // compilation failed!
        GL_CHECK_RET(glGetShaderiv(mShader, GL_INFO_LOG_LENGTH, &retval),
                     false);

        gLog.err("compilation failed! log:");
        if (retval > 0)
        {
            std::string buffer;
            buffer.resize(retval);
            GL_CHECK_RET(glGetShaderInfoLog(mShader, retval - 1,
                                            &retval, &buffer[0]), false);
            gLog.printf("%s", buffer.c_str());
        }

        return false;
    }

    return true;
}

Shader::Shader(const std::shared_ptr<ConcreteShader>& vertex,
               const std::shared_ptr<ConcreteShader>& fragment,
               const std::shared_ptr<ConcreteShader>& geometry):
    mProgram(linkShader(vertex, fragment, geometry)),
    mInputs(vertex->getInputs())
{}

ProgramId Shader::linkShader(const std::shared_ptr<ConcreteShader>& vertex,
                             const std::shared_ptr<ConcreteShader>& fragment,
                             const std::shared_ptr<ConcreteShader>& geometry)
{
    ProgramId id;
    GL_CHECK(id = glCreateProgram());
    if (!id) {
        return 0;
    }

    if (vertex) {
        GL_CHECK(glAttachShader(id, vertex->getShader()));
    }
    if (fragment) {
        GL_CHECK(glAttachShader(id, fragment->getShader()));
    }
    if (geometry) {
        GL_CHECK(glAttachShader(id, geometry->getShader()));
    }

    gLog.trace("linking shader program...");
    GL_CHECK(glLinkProgram(id));

    if (!shaderLinkSucceeded(id)) {
        gLog.err("shader link failed");
        return 0;
    }

    return id;
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

#define DEFINE_UNIFORM_SETTER(Type, GLType, glSetter, ...) \
    bool Shader::setUniform(const char* name, const Type* value_array, uint32_t elements) \
    { \
        if (!mProgram) return false; \
        GLint loc = glGetUniformLocation(mProgram, name); \
        if (loc == -1) return false; \
        GL_CHECK(glSetter(loc, elements, ##__VA_ARGS__, (const GLType*)value_array)); \
        return true; \
    }

DEFINE_UNIFORM_SETTER(float, GLfloat, glUniform1fv)
DEFINE_UNIFORM_SETTER(Vec2, GLfloat, glUniform2fv)
DEFINE_UNIFORM_SETTER(Vec3, GLfloat, glUniform3fv)
DEFINE_UNIFORM_SETTER(Color, GLfloat, glUniform4fv)
DEFINE_UNIFORM_SETTER(int, GLint, glUniform1iv)

DEFINE_UNIFORM_SETTER(Mat44, GLfloat, glUniformMatrix4fv, GL_FALSE)

void Shader::bind(const VertexBuffer& vb)
{
    gLog.debug("bind program %d\n", mProgram);
    GL_CHECK(glUseProgram(mProgram));

    GLuint i = 0;
    for (const BufferKindPair& pair: vb.getBuffers()) {
        const Input& input = mInputs[pair.kind];

        gLog.debug("- enable attrib %lu (%s)\n", i, input.name.c_str());
        GL_CHECK(glBindAttribLocation(mProgram, i, input.name.c_str()));
        ++i;
    }

    if (i != mInputs.size()) {
        std::vector<std::string> expected;
        std::vector<std::string> actual;

        std::transform(mInputs.begin(), mInputs.end(),
                       std::back_inserter(expected),
                       [](const std::pair<Attrib::Kind, Input>& p) {
                           return ATTRIBS.find(p.first)->second.kindAsString;
                       });
        std::transform(vb.getBuffers().begin(), vb.getBuffers().end(),
                       std::back_inserter(actual),
                       [](const BufferKindPair& p) {
                           return ATTRIBS.find(p.kind)->second.kindAsString;
                       });
        sbFail("%s", utils::format(
                   "not all inputs available in buffer, expected {0}, got {1}",
                   utils::join(expected, ", "),
                   utils::join(actual, ", ")).c_str());
    }
}

void Shader::unbind()
{
    gLog.debug("unbind program\n");
    GL_CHECK(glUseProgram(0));
}

} // namespace sb

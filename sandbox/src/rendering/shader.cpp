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

const std::map<std::string, Attrib::Kind> ATTRIB_KINDS {
    { "POSITION", Attrib::Kind::Position },
    { "TEXCOORD", Attrib::Kind::Texcoord },
    { "COLOR", Attrib::Kind::Color },
    { "NORMAL", Attrib::Kind::Normal },
    { "TANGENT", Attrib::Kind::Tangent },
    { "BITANGENT", Attrib::Kind::Bitangent },
    { "", Attrib::Kind::Unspecified }
};

ssize_t extractLineNum(const std::string& logLine)
{
    size_t numStart = logLine.find_first_not_of("0123456789");
    if (numStart == std::string::npos) {
        return -1;
    }

    ++numStart;
    size_t numEnd = logLine.find_first_not_of("0123456789", numStart);

    return lexical_cast<ssize_t>(logLine.substr(numStart, numEnd - numStart));
}

void printWithContext(const PreprocessedCode& source,
                      size_t lineNum,
                      size_t contextLines = 3)
{
    size_t start = (size_t)std::max((ssize_t)lineNum - (ssize_t)contextLines,
                                    (ssize_t)0);
    size_t end = std::min(lineNum + contextLines + 1, source.getNumLines() - 1);

    const auto backtrace = source.findLine(lineNum).getBacktrace();
    for (auto it = backtrace.rbegin(); it != --backtrace.rend(); ++it) {
        gLog.printf("included from %s:%u", it->file.c_str(), it->line);
    }
    gLog.printf("     error at %s:%u",
                backtrace.front().file.c_str(), backtrace.front().line);
    gLog.printf("--- code ---");

    for (size_t i = start; i < end; ++i) {
        gLog.printf("%s % 4u: %s\n",
                    (i == lineNum ? ">>" : "  "), (unsigned)(i + 1),
                    source.findLine(i)->getLine().c_str());
    }
}

void printPreprocessedCompileLog(const std::string& log,
                                 const PreprocessedCode& source)
{
    std::vector<std::string> logLines = utils::split(log, "\n");

    ssize_t lineNum = -1;
    for (const auto& logLine: logLines) {
        ssize_t newLineNum = extractLineNum(logLine);
        if (newLineNum >= 0 && newLineNum != lineNum) {
            if (lineNum >= 0) {
                printWithContext(source, (size_t)lineNum - 1);
            }
            lineNum = newLineNum;
        }
        gLog.printf("%s", logLine.c_str());
    }

    if (lineNum >= 0) {
        printWithContext(source, (size_t)lineNum - 1);
    }
}

bool isInputLine(const std::string& line,
                 const std::vector<std::string>& words,
                 bool warnOnUntagged) {
    if (words.size() == 0
            || words[0] != "in") {
        return false;
    }

    if (words.size() < 5) {
        if (warnOnUntagged) {
            std::vector<std::string> tags;
            std::transform(ATTRIB_KINDS.begin(), ATTRIB_KINDS.end(),
                           std::back_inserter(tags),
                           [](const std::pair<std::string, Attrib::Kind>& p) {
                               return p.first;
                           });

            gLog.warn("untagged input: %s", line.c_str());
            gLog.warn("expected format: in <type> <name> // TAG");
            gLog.warn("recognized tags: %s", utils::join(tags, ", ").c_str());
        }
    }

    return true;
}

void extractInput(std::set<Input>& outInputs,
                  const std::string& line,
                  bool warnOnUntagged)
{
    std::vector<std::string> words = utils::split(line);

    if (!isInputLine(line, words, warnOnUntagged)) {
        return;
    }

    const std::string& type = words[1];
    const std::string& name = utils::strip(words[2], "[]0123456789;");
    const std::string& kind_str = words.size() > 4 ? words[4] : "";

    auto kindIt = ATTRIB_KINDS.find(kind_str);
    if (kindIt == ATTRIB_KINDS.end()) {
        sbFail("unknown attribute kind: %s", kind_str.c_str());
    }
    const Attrib::Kind kind = kindIt->second;

    if (kind != Attrib::Kind::Unspecified) {
        auto it = std::find_if(outInputs.begin(), outInputs.end(),
                               [kind](const Input& i) {
                                   return i.kind == kind;
                               });
        if (it != outInputs.end()) {
            sbFail("multiple inputs of the same kind (%s) detected: %s and %s",
                   kind_str.c_str(), it->name.c_str(), name.c_str());
        }

        gLog.trace("input (%s): %s %s",
                   kind_str.c_str(), type.c_str(), name.c_str());
    } else {
        gLog.trace("input: %s %s", type.c_str(), name.c_str());
    }

    outInputs.insert(Input(name, type, kind));
}

void extractOutput(std::set<Output>& outOutputs,
                   const std::string& line)
{
    std::vector<std::string> words = utils::split(line);

    if (words.size() < 3
            || words[0] != "out") {
        return;
    }

    const std::string& type = words[1];
    const std::string& name = utils::strip(words[2], "[]0123456789;");

    gLog.trace("output: %s %s", type.c_str(), name.c_str());

    if (outOutputs.find(name) != outOutputs.end()) {
        sbFail("multiple outputs with same name (%s) detected", name.c_str());
    }

    outOutputs.insert(Output(name, type));
}

void detectOptimizedOutUniforms(GLuint program,
                                std::set<Uniform>& uniforms)
{
    for (const Uniform& uniform: uniforms) {
        std::string nameToCheck = uniform.name;
        if (uniform.type.size() > 2
                && uniform.type.substr(uniform.type.size() - 2) == "[]") {
            nameToCheck += "[0]";
        }

        if (glGetUniformLocation(program, nameToCheck.c_str()) == -1) {
            gLog.warn("uniform \"%s\" may be optimized out!", uniform.name.c_str());
        }
    }
}

} // namespace

std::set<Input> ConcreteShader::parseInputs(bool warnOnUntagged)
{
    std::set<Input> ret;

    for (const auto& node: mPreprocessedSource) {
        if (node.isLine()) {
            extractInput(ret, node.getLine(), warnOnUntagged);
        }
    }

    return ret;
}

std::set<Output> ConcreteShader::parseOutputs()
{
    std::set<Output> ret;

    for (const auto& node: mPreprocessedSource) {
        if (node.isLine()) {
            extractOutput(ret, node.getLine());
        }
    }

    return ret;
}

std::set<Uniform> ConcreteShader::parseUniforms()
{
    std::set<Uniform> ret;

    for (const auto& node: mPreprocessedSource) {
        if (!node.isLine()) {
            continue;
        }

        std::vector<std::string> words = utils::split(node.getLine());

        if (words.size() > 2
                && words[0] == "uniform") {
            std::string uniformType = utils::strip(words[1]);
            if (words[2].find("[") != std::string::npos
                    || (words.size() > 3 && words[3][0] == '[')) {
                uniformType += "[]";
            }

            std::string uniformName =
                    utils::strip(
                        utils::split(utils::strip(words[2], ";"), "[")[0]);

            ret.insert(Uniform(uniformName, uniformType));
            gLog.trace("uniform: %s %s",
                       uniformType.c_str(), uniformName.c_str());
        }
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
            printPreprocessedCompileLog(buffer, mPreprocessedSource);
        }

        return false;
    }

    return true;
}

namespace {

void checkShaderCompatibility(
    const std::shared_ptr<ConcreteShader>& first,
    const std::shared_ptr<ConcreteShader>& second)
{
    const std::set<Output>& outputs = first->getOutputs();

    gLog.debug("checking compat: %s, %s", first->getFilename().c_str(),
               second->getFilename().c_str());
    gLog.debug("outputs:");
    for (const Output& out: outputs) {
        gLog.debug("- %s %s", out.type.c_str(), out.name.c_str());
    }
    gLog.debug("inputs:");
    for (const Input& in: second->getInputs()) {
        gLog.debug("- %s %s", in.type.c_str(), in.name.c_str());
    }

    for (const Input& input: second->getInputs()) {
        gLog.debug("checking input %s", input.name.c_str());

        auto outputIt = outputs.find(Output(input.name));
        if (outputIt == outputs.end()) {
            sbFail("dangling input: %s in shader %s when used with %s",
                   input.name.c_str(), second->getFilename().c_str(),
                   first->getFilename().c_str());
        }

        if (input.type != outputIt->type) {
            sbFail("different types for in/out variable %s (%s, %s) in shaders "
                   "%s and %s", input.name.c_str(), input.type.c_str(),
                   outputIt->type.c_str(), first->getFilename().c_str(),
                   second->getFilename().c_str());
        }
    }
}

void checkInputOutputCompatbility(
        const std::shared_ptr<ConcreteShader>& vertex,
        const std::shared_ptr<ConcreteShader>& fragment,
        const std::shared_ptr<ConcreteShader>& geometry)
{
    if (geometry) {
        checkShaderCompatibility(vertex, geometry);
        checkShaderCompatibility(geometry, fragment);
    } else {
        checkShaderCompatibility(vertex, fragment);
    }
}

} // namespace

Shader::Shader(const std::shared_ptr<ConcreteShader>& vertex,
               const std::shared_ptr<ConcreteShader>& fragment,
               const std::shared_ptr<ConcreteShader>& geometry):
    mVertexShader(vertex),
    mFragmentShader(fragment),
    mGeometryShader(geometry),
    mProgram(linkShader(vertex, fragment, geometry)),
    mFilenames({ vertex->getFilename(), fragment->getFilename() }),
    mInputs(vertex->makeInputsMap())
{
    checkInputOutputCompatbility(vertex, fragment, geometry);

    for (const Uniform& uniform: vertex->getUniforms()) {
        mUniforms.insert(uniform);
    }
    for (const Uniform& uniform: fragment->getUniforms()) {
        mUniforms.insert(uniform);
    }

    if (geometry) {
        for (const Uniform& uniform: geometry->getUniforms()) {
            mUniforms.insert(uniform);
        }
        mFilenames.push_back(geometry->getFilename());
    }

    detectOptimizedOutUniforms(mProgram, mUniforms);
}

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
        sbFail("shader link failed");
        return 0;
    }

    return id;
}

namespace {

void printSource(const PreprocessedCode& code)
{
    std::string currFile = "";
    size_t currLine = 0;

    for (auto it = code.begin(); it != code.end(); ++it) {
        const std::vector<BacktraceNode> backtrace = it.getBacktrace();
        if (currFile != backtrace[0].file) {
            currFile = backtrace[0].file;
            gLog.printf("// file: %s", currFile.c_str());
        }

        gLog.printf("% 4u: %s\n", (unsigned)(currLine + 1),
                    it->getLine().c_str());
        ++currLine;
    }
}

} // namespace

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

            if (mVertexShader) {
                gLog.info("vertex shader code:");
                printSource(mVertexShader->getPreprocessedSource());
            }
            if (mGeometryShader) {
                gLog.info("geometry shader code:");
                printSource(mGeometryShader->getPreprocessedSource());
            }
            if (mFragmentShader) {
                gLog.info("fragment shader code:");
                printSource(mGeometryShader->getPreprocessedSource());
            }
        }

        return false;
    }

    return true;
}

#define DEFINE_UNIFORM_SETTER(Type, GLType, glSetter, ...) \
    bool Shader::setUniform(const char* name, \
                            const Type* value_array, \
                            uint32_t elements) const \
    { \
        if (!mProgram) { \
            sbFail("invalid program: %d", (int)mProgram); \
            return false; \
        } \
        GLint loc; \
        GL_CHECK(loc = glGetUniformLocation(mProgram, name)); \
        if (loc == -1) { \
            std::string name_str = \
                    utils::split(utils::split(name, ".")[0], "[")[0]; \
            if (!hasUniform(name_str)) { \
                sbFail("no uniform \"%s\" in shader: %s", \
                       name, getName().c_str()); \
            } else { \
                sbFail("uniform \"%s\" was optimized out by the shader " \
                       "compiler in shader: %s", \
                       name, getName().c_str()); \
            } \
            return false; \
        } \
        GL_CHECK(glSetter(loc, elements, ##__VA_ARGS__, (const GLType*)value_array)); \
        return true; \
    }

DEFINE_UNIFORM_SETTER(float, GLfloat, glUniform1fv)
DEFINE_UNIFORM_SETTER(Vec2, GLfloat, glUniform2fv)
DEFINE_UNIFORM_SETTER(Vec3, GLfloat, glUniform3fv)
DEFINE_UNIFORM_SETTER(Color, GLfloat, glUniform4fv)
DEFINE_UNIFORM_SETTER(int, GLint, glUniform1iv)
DEFINE_UNIFORM_SETTER(unsigned, GLuint, glUniform1uiv)

DEFINE_UNIFORM_SETTER(Mat44, GLfloat, glUniformMatrix4fv, GL_FALSE)
DEFINE_UNIFORM_SETTER(Mat33, GLfloat, glUniformMatrix3fv, GL_FALSE)

void Shader::bind(const VertexBuffer& vb) const
{
    GL_CHECK(glUseProgram(mProgram));

    size_t bound = 0;
    GLuint i = 0;
    for (const BufferKindPair& pair: vb.getBuffers()) {
        auto inputIt = mInputs.find(pair.kind);

        if (inputIt != mInputs.end()) {
            const Input& input = inputIt->second;
            GL_CHECK(glBindAttribLocation(mProgram, i, input.name.c_str()));
            ++bound;
        }

        ++i;
    }

    if (bound < mInputs.size()) {
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
                   "not all inputs available in buffer, expected:\n{0}\ngot\n{1}",
                   utils::join(expected, ", "),
                   utils::join(actual, ", ")).c_str());
    }
}

void Shader::unbind() const
{
    GL_CHECK(glUseProgram(0));
}

} // namespace sb

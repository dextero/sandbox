#ifndef SHADER_H
#define SHADER_H

#include <vector>
#include <string>
#include <map>
#include <set>
#include <memory>

#include "types.h"
#include "color.h"
#include "vertexBuffer.h"
#include "../utils/types.h"
#include "../utils/stringUtils.h"
#include "../utils/lib.h"
#include "utils/code_preprocessor.h"

namespace sb
{
    struct Input
    {
        Attrib::Kind kind;
        std::string type;
        std::string name;

        Input(const std::string& name,
              const std::string& type = "",
              Attrib::Kind kind = Attrib::Kind::Unspecified):
            kind(kind),
            type(type),
            name(name)
        {}

        bool operator <(const Input& i) const {
            return name < i.name;
        }
    };

    struct Output
    {
        std::string type;
        std::string name;

        Output(const std::string& name,
               const std::string& type = ""):
            type(type),
            name(name)
        {}

        bool operator <(const Output& o) const {
            return name < o.name;
        }
    };

    struct Uniform
    {
        std::string name;
        std::string type;

        bool operator <(const Uniform& u) const {
            return name < u.name;
        }

        Uniform(const std::string& name,
                const std::string& type = ""):
            name(name),
            type(type)
        {}
    };

    class ConcreteShader
    {
    public:
        ConcreteShader(GLuint shaderType,
                       const std::string& path):
            mShader(0),
            mFilename(path)
        {
            mPreprocessedSource = PreprocessedCode(path);

            GL_CHECK(mShader = glCreateShader(shaderType));

            std::string code = mPreprocessedSource.getCode();
            //gLog.debug("code:\n%s", code.c_str());
            const GLchar* codePtr = (const GLchar*)&code[0];
            GL_CHECK(glShaderSource(mShader, 1, &codePtr, NULL));

            static std::map<GLuint, std::string> SHADERS {
                { GL_VERTEX_SHADER, "vertex" },
                { GL_FRAGMENT_SHADER, "fragment" },
                { GL_GEOMETRY_SHADER, "geometry" }
            };

            gLog.trace("compiling %s shader: %s",
                       SHADERS[shaderType].c_str(), path.c_str());
            GL_CHECK(glCompileShader(mShader));
            if (!shaderCompilationSucceeded()) {
                sbFail("shader compilation failed");
            }

            mInputs = parseInputs(shaderType == GL_VERTEX_SHADER);
            mOutputs = parseOutputs();
            mUniforms = parseUniforms();
        }

        ~ConcreteShader()
        {
            if (mShader) {
                GL_CHECK(glDeleteShader(mShader));
            }
        }

        GLuint getShader() const { return mShader; }
        const std::set<Input>& getInputs() const { return mInputs; }
        const std::set<Output> getOutputs() const { return mOutputs; }
        const std::set<Uniform> getUniforms() const { return mUniforms; }
        const std::string& getFilename() const { return mFilename; }
        const PreprocessedCode& getPreprocessedSource() const { return mPreprocessedSource; }

        std::map<Attrib::Kind, Input> makeInputsMap() const {
            std::map<Attrib::Kind, Input> ret;

            for (const Input& input: mInputs) {
                if (input.kind == Attrib::Kind::Unspecified) {
                    sbFail("input %s is missing a kind annotation in shader %s",
                           input.name.c_str(), mFilename.c_str());
                }

                ret.insert({ input.kind, input });
            }

            return ret;
        }

    private:
        bool shaderCompilationSucceeded();
        std::set<Input> parseInputs(bool warnOnUntagged);
        std::set<Output> parseOutputs();
        std::set<Uniform> parseUniforms();

        GLuint mShader;
        std::string mFilename;
        std::set<Input> mInputs;
        std::set<Output> mOutputs;
        std::set<Uniform> mUniforms;
        PreprocessedCode mPreprocessedSource;
    };

    class Shader
    {
    public:
        template<typename T>
        bool setUniform(const char* name,
                        const T& value) const
        {
            return setUniform(name, &value, 1);
        }

        template<typename T>
        bool setUniform(const char*, const T*, uint32_t) const
        {
            sbFail("Wrong overload called!");
        }

        bool setUniform(const char* name,
                        const float* value_array,
                        uint32_t elements) const;
        bool setUniform(const char* name,
                        const Vec2* value_array,
                        uint32_t elements) const;
        bool setUniform(const char* name,
                        const Vec3* value_array,
                        uint32_t elements) const;
        bool setUniform(const char* name,
                        const Color* value_array,
                        uint32_t elements) const;
        bool setUniform(const char* name,
                        const Mat44* value_array,
                        uint32_t elements) const;
        bool setUniform(const char* name,
                        const Mat33* value_array,
                        uint32_t elements) const;
        bool setUniform(const char* name,
                        const int* value_array,
                        uint32_t elements) const;
        bool setUniform(const char* name,
                        const unsigned* value_array,
                        uint32_t elements) const;

        template<typename T>
        inline bool setUniform(const std::string& name,
                               const T& value) const
        {
            return setUniform(name.c_str(), value);
        }

        template<typename T>
        inline bool setUniform(const std::string& name,
                               const T* value_array,
                               uint32_t elements) const
        {
            return setUniform(name.c_str(), value_array, elements);
        }

        enum ESamplerType {
            SamplerImage = 0,
            SamplerNormalmap = 1,
            SamplerShadowmap = 2
        };

        void bind(const VertexBuffer& vb) const;
        void unbind() const;

        const std::map<Attrib::Kind, Input>& getInputs() const {
            return mInputs;
        }

        bool hasUniform(const std::string& name) const {
            return mUniforms.count(name) > 0;
        }

        Shader(Shader&& prev) { *this = std::move(prev); }
        Shader& operator =(Shader&& prev)
        {
            mProgram = prev.mProgram;
            prev.mProgram = 0;
            mFilenames.swap(prev.mFilenames);
            mInputs.swap(prev.mInputs);
            mUniforms.swap(prev.mUniforms);
            return *this;
        }

        ~Shader()
        {
            if (mProgram) {
                GL_CHECK(glDeleteProgram(mProgram));
            }
        }

        std::string getName() const {
            return utils::join(mFilenames, ", ");
        }

    private:
        std::shared_ptr<ConcreteShader> mVertexShader;
        std::shared_ptr<ConcreteShader> mFragmentShader;
        std::shared_ptr<ConcreteShader> mGeometryShader;
        ProgramId mProgram;
        std::vector<std::string> mFilenames;
        std::map<Attrib::Kind, Input> mInputs;
        std::set<Uniform> mUniforms;

        Shader(const std::shared_ptr<ConcreteShader>& vertex,
               const std::shared_ptr<ConcreteShader>& fragment,
               const std::shared_ptr<ConcreteShader>& geometry);

        ProgramId linkShader(const std::shared_ptr<ConcreteShader>& vertex,
                             const std::shared_ptr<ConcreteShader>& fragment,
                             const std::shared_ptr<ConcreteShader>& geometry);

        bool shaderLinkSucceeded(ProgramId program);

        friend class ResourceMgr;
    };
} // namespace sb

#endif //SHADER_H

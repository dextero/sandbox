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

namespace sb
{
    struct Input
    {
        Attrib::Kind kind;
        std::string type;
        std::string name;
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
            std::string code = utils::readFile(path);
            GL_CHECK(mShader = glCreateShader(shaderType));

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
            if (!shaderCompilationSucceeded(code)) {
                sbFail("shader compilation failed");
            }

            mInputs = parseInputs(code, shaderType == GL_VERTEX_SHADER);
            mUniforms = parseUniforms(code);
        }

        ~ConcreteShader()
        {
            if (mShader) {
                GL_CHECK(glDeleteShader(mShader));
            }
        }

        GLuint getShader() const { return mShader; }
        const std::map<Attrib::Kind, Input>& getInputs() const { return mInputs; }
        const std::set<Uniform> getUniforms() const { return mUniforms; }
        const std::string& getFilename() const { return mFilename; }

    private:
        bool shaderCompilationSucceeded(const std::string& source);
        static std::map<Attrib::Kind, Input> parseInputs(const std::string& code,
                                                         bool warnOnUntagged);
        static std::set<Uniform> parseUniforms(const std::string& code);

        GLuint mShader;
        std::string mFilename;
        std::map<Attrib::Kind, Input> mInputs;
        std::set<Uniform> mUniforms;
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

        static bool shaderLinkSucceeded(ProgramId program);

        friend class ResourceMgr;
    };
} // namespace sb

#endif //SHADER_H

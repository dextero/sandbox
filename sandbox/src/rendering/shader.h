#ifndef SHADER_H
#define SHADER_H

#include <vector>
#include <string>
#include <map>
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

    class ConcreteShader
    {
    public:
        ConcreteShader(GLuint shaderType,
                       const std::string& path):
            mShader(0)
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
        }

        ~ConcreteShader()
        {
            if (mShader) {
                GL_CHECK(glDeleteShader(mShader));
            }
        }

        GLuint getShader() const { return mShader; }
        const std::map<Attrib::Kind, Input>& getInputs() const
        {
            return mInputs;
        }

    private:
        bool shaderCompilationSucceeded(const std::string& source);
        static std::map<Attrib::Kind, Input> parseInputs(const std::string& code,
                                                         bool warnOnUntagged);

        GLuint mShader;
        std::map<Attrib::Kind, Input> mInputs;
    };

    class Shader
    {
    public:
        template<typename T>
        bool setUniform(const char* name,
                        const T& value)
        {
            return setUniform(name, &value, 1);
        }

        template<typename T>
        bool setUniform(const char*, const T*, uint32_t)
        {
            sbFail("Wrong overload called!");
        }

        bool setUniform(const char* name,
                        const float* value_array,
                        uint32_t elements);
        bool setUniform(const char* name,
                        const Vec2* value_array,
                        uint32_t elements);
        bool setUniform(const char* name,
                        const Vec3* value_array,
                        uint32_t elements);
        bool setUniform(const char* name,
                        const Color* value_array,
                        uint32_t elements);
        bool setUniform(const char* name,
                        const Mat44* value_array,
                        uint32_t elements);
        bool setUniform(const char* name,
                        const int* value_array,
                        uint32_t elements);

        enum ESamplerType {
            SamplerImage = 0,
            SamplerNormalmap = 1,
            SamplerShadowmap = 2
        };

        void bind(const VertexBuffer& vb);
        void unbind();

        const std::map<Attrib::Kind, Input>& getInputs() {
            return mInputs;
        }

        Shader(Shader&& prev) { *this = std::move(prev); }
        Shader& operator =(Shader&& prev)
        {
            mProgram = prev.mProgram;
            prev.mProgram = 0;
            mInputs.swap(prev.mInputs);
            return *this;
        }

        ~Shader()
        {
            if (mProgram) {
                GL_CHECK(glDeleteProgram(mProgram));
            }
        }

    private:
        ProgramId mProgram;
        std::map<Attrib::Kind, Input> mInputs;

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

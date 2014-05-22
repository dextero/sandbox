#ifndef SHADER_H
#define SHADER_H

#include <vector>
#include <string>
#include <map>

#include "types.h"
#include "color.h"
#include "../utils/types.h"

namespace sb
{
    class Shader
    {
    private:
        ProgramId mProgram;

        enum EShaderType {
            ShaderTypeVertex = 0,
            ShaderTypeFragment,
            ShaderTypeGeometry,

            ShaderTypesCount = 3
        };
        ShaderId mShaders[ShaderTypesCount];

        static GLenum translateShaderType(EShaderType type);
        static bool checkCompilationStatus(ShaderId shader);
        static bool checkLinkStatus(ProgramId program);
    public:
        Shader();
        ~Shader();

        bool loadShader(EShaderType type, const char* file);
        bool load(const char* vertex, const char* fragment, const char* geometry = NULL);

        bool compileAndLink();
        void free();

        template<typename T> bool setUniform(const char* name, const T& value)
        {
            return setUniform(name, &value, 1);
        }

        template<typename T> bool setUniform(const char* name, const T* value_array, uint32_t elements)
        {
            (void)name;
            (void)value_array;
            (void)elements;
            assert(!"Wrong overload called!");
            return false;
        }

        bool setUniform(const char* name, const float* value_array, uint32_t elements);
        bool setUniform(const char* name, const Vec2* value_array, uint32_t elements);
        bool setUniform(const char* name, const Vec3* value_array, uint32_t elements);
        bool setUniform(const char* name, const Color* value_array, uint32_t elements);
        bool setUniform(const char* name, const Mat44* value_array, uint32_t elements);
        bool setUniform(const char* name, const int* value_array, uint32_t elements);

        enum ESamplerType {
            SamplerImage = 0,
            SamplerNormalmap = 1,
            SamplerShadowmap = 2
        };

        ProgramId getProgram() { return mProgram; }

        // --------------------------------
        enum EShader {
            ShaderNone = 0,
            ShaderTexture,
            ShaderColor,
            ShaderPointSprite,

            ShaderCount
        };
        static std::vector<Shader> msShaders;
        static std::vector<std::vector<std::pair<uint32_t, std::string> > > msAttribs;
        static EShader msCurrent;

        static void initShaders();
        static void freeShaders();
        static Shader& get(EShader shader);
        static Shader& getCurrent();

        static void use(EShader shader);
    };

    #define    CurrShader    Shader::getCurrent()
} // namespace sb

#endif //SHADER_H

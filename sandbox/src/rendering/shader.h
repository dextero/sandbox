#ifndef SHADER_H
#define SHADER_H

#include "types.h"
#include "color.h"
#include "../utils/types.h"
#include "../utils/profiler.h"

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

        static GLenum TranslateShaderType(EShaderType type);
        static bool CheckCompilationStatus(ShaderId shader);
        static bool CheckLinkStatus(ProgramId program);
    public:
        Shader();
        ~Shader();

        bool LoadShader(EShaderType type, const char* file);
        bool Load(const char* vertex, const char* fragment, const char* geometry = NULL);

        bool CompileAndLink();
        void Free();

        template<typename T> bool SetUniform(const char* name, const T& value)
        {
            PROFILE();

            return SetUniform(name, &value, 1);
        }

        template<typename T> bool SetUniform(const char* name, const T* value_array, uint elements)
        {
            (void)name;
            (void)value_array;
            (void)elements;
            assert(!"Wrong overload called!");
            return false;
        }

        bool SetUniform(const char* name, const float* value_array, uint elements);
        bool SetUniform(const char* name, const Vec2* value_array, uint elements);
        bool SetUniform(const char* name, const Vec3* value_array, uint elements);
        bool SetUniform(const char* name, const Color* value_array, uint elements);
        bool SetUniform(const char* name, const Mat44* value_array, uint elements);
        bool SetUniform(const char* name, const int* value_array, uint elements);

        enum ESamplerType {
            SamplerImage = 0,
            SamplerNormalmap = 1,
            SamplerShadowmap = 2
        };

        ProgramId GetProgram() { return mProgram; }

        // --------------------------------
        enum EShader {
            ShaderNone = 0,
            ShaderTexture,
            ShaderColor,
            ShaderPointSprite,

            ShaderCount
        };
        static std::vector<Shader> msShaders;
        static std::vector<std::vector<std::pair<uint, std::string> > > msAttribs;
        static EShader msCurrent;

        static void InitShaders();
        static void FreeShaders();
        static Shader& Get(EShader shader);
        static Shader& GetCurrent();

        static void Use(EShader shader);
    };

    #define    CurrShader    Shader::GetCurrent()
} // namespace sb

#endif //SHADER_H

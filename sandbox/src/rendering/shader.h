#ifndef SHADER_H
#define SHADER_H

#include <vector>
#include <string>
#include <map>
#include <memory>

#include "types.h"
#include "color.h"
#include "../utils/types.h"

namespace sb
{
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
        bool setUniform(const char* name,
                        const T* value_array,
                        uint32_t elements)
        {
            (void)name;
            (void)value_array;
            (void)elements;
            assert(!"Wrong overload called!");
            return false;
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

        void bind();
        void unbind();

    private:
        std::shared_ptr<ProgramId> mProgram;
        std::shared_ptr<ShaderId> mVertexShader;
        std::shared_ptr<ShaderId> mFragmentShader;
        std::shared_ptr<ShaderId> mGeometryShader;

        std::vector<std::string> mAttribs;

        Shader(const std::shared_ptr<ShaderId>& vertex,
               const std::shared_ptr<ShaderId>& fragment,
               const std::shared_ptr<ShaderId>& geometry,
               const std::vector<std::string>& attribs);

        std::shared_ptr<ProgramId>
        linkShader(const std::shared_ptr<ShaderId>& vertex,
                   const std::shared_ptr<ShaderId>& fragment,
                   const std::shared_ptr<ShaderId>& geometry);

        static bool shaderLinkSucceeded(ProgramId program);

        friend class ResourceMgr;
    };
} // namespace sb

#endif //SHADER_H

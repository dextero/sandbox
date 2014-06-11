#include "utils/math.h"

namespace sb
{
    namespace math
    {
        Mat44 matrixOrthographic(float left,
                                 float right,
                                 float bottom,
                                 float top,
                                 float near,
                                 float far)
        {
            return glm::ortho(left, right, bottom, top, near, far);
        }

        Mat44 matrixPerspective(float fov,
                                float aspectRatio,
                                float near,
                                float far)
        {
            return glm::perspective(fov, aspectRatio, near, far);
        }
 
        namespace detail
        {
            uint32_t nextPowerOf2<uint32_t>::operator ()(uint32_t value)
            {
                value--;
                value |= value >> 1;
                value |= value >> 2;
                value |= value >> 4;
                value |= value >> 8;
                value |= value >> 16;
                value++;
                return value;
            }

            uint64_t nextPowerOf2<uint64_t>::operator ()(uint64_t value)
            {
                value--;
                value |= value >> 1;
                value |= value >> 2;
                value |= value >> 4;
                value |= value >> 8;
                value |= value >> 16;
                value |= value >> 32;
                value++;
                return value;
            }
        }
    } // namespace Math
} // namespace sb

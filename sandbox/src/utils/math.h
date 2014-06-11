#ifndef MATHUTILS_H
#define MATHUTILS_H

#include "types.h"

#ifdef near
#   undef near
#endif
#ifdef far
#   undef far
#endif

namespace sb
{
    namespace math
    {
        static const float EPSILON = 0.0001f;

        template<typename T> T clamp(T value, T min, T max)
        {
            return (value < min ? min : (value > max ? max : value));
        }

        Mat44 matrixOrthographic(float left,
                                 float right,
                                 float bottom,
                                 float top,
                                 float near = Z_NEAR,
                                 float far = Z_FAR);
        Mat44 matrixPerspective(float fov,
                                float aspectRatio,
                                float near = Z_NEAR,
                                float far = Z_FAR);

        namespace detail
        {
            template<typename T>
            struct nextPowerOf2;

            template<>
            struct nextPowerOf2<uint32_t>
            {
                uint32_t operator()(uint32_t value);
            };

            template<>
            struct nextPowerOf2<uint64_t>
            {
                uint64_t operator()(uint64_t value);
            };
        }

        template<typename T>
        T nextPowerOf2(T value)
        {
            return detail::nextPowerOf2<T>()(value);
        }
    } // namespace Math
} // namespace sb

#endif //MATHUTILS_H

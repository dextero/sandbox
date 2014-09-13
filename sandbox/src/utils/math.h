#ifndef MATHUTILS_H
#define MATHUTILS_H

#include "types.h"

#ifdef near
#   undef near
#endif
#ifdef far
#   undef far
#endif

namespace sb {
namespace math {

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
Mat44 matrixShadowBias();

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

// bilinear interpolation
template<typename T>
T bilerp(const T &x,
         const T &y,
         const T &c00,
         const T &c10,
         const T &c01,
         const T &c11)
{
#if 1
    T a = c00 * (T(1) - x) + c10 * x;
    T b = c01 * (T(1) - x) + c11 * x;
    return a * (T(1) - y) + b * y;
#else
    return (T(1) - x) * (T(1) - y) * c00 +
            x * (T(1) - y) * c10 +
            (T(1) - x) * y * c01 +
            x * y * c11;
#endif
}

} // namespace math
} // namespace sb

std::ostream& operator <<(std::ostream& os,
                          const Mat44& mat);

#endif //MATHUTILS_H

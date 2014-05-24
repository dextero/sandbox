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
    } // namespace Math
} // namespace sb

#endif //MATHUTILS_H

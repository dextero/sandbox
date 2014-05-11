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
    namespace Math
    {
        template<typename T> T Clamp(T value, T min, T max)
        {
            return (value < min ? min : (value > max ? max : value));
        }

        Mat44 MatrixOrthographic(float left,
                                 float right,
                                 float bottom,
                                 float top,
                                 float near = Z_NEAR,
                                 float far = Z_FAR);
        Mat44 MatrixPerspective(float fov,
                                float aspectRatio,
                                float near = Z_NEAR,
                                float far = Z_FAR);
    } // namespace Math
} // namespace sb

#endif //MATHUTILS_H

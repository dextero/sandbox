#include "mathUtils.h"

namespace sb
{
    namespace Math
    {
        Mat44 MatrixOrthographic(float left, float right, float bottom, float top, float near, float far)
        {
            return glm::ortho(left, right, bottom, top, near, far);
        }

        Mat44 MatrixPerspective(float fov, float aspectRatio, float near, float far)
        {
            return glm::perspective(fov, aspectRatio, near, far);
        }
    } // namespace Math
} // namespace sb

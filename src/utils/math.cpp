#include <sandbox/utils/math.h>
#include <sandbox/utils/lexical_cast.h>

#include <iomanip>

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

        Mat44 matrixShadowBias()
        {
            return {
#if 1
                0.5f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.5f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.5f, 0.0f,
                0.5f, 0.5f, 0.5f, 1.0f,
#else
                0.5f, 0.0f, 0.0f, 0.5f,
                0.0f, 0.5f, 0.0f, 0.5f,
                0.0f, 0.0f, 0.5f, 0.5f,
                0.0f, 0.0f, 0.0f, 1.0f,
#endif
            };
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

std::ostream& operator <<(std::ostream& os,
                          const sb::Mat44& mat)
{
    constexpr std::streamsize PRECISION = 8;

    const std::streamsize oldPrecision = os.precision();

#define MAT(col, row) \
    std::setw(PRECISION + 1) \
    << lexical_cast<std::string>(mat[col][row]).substr(0, PRECISION)

    os << ".' "  << MAT(0, 0) << MAT(1, 0) << MAT(2, 0) << MAT(3, 0) << " '.\n"
       << "|  "  << MAT(0, 1) << MAT(1, 1) << MAT(2, 1) << MAT(3, 1) << "  |\n"
       << "|  "  << MAT(0, 2) << MAT(1, 2) << MAT(2, 2) << MAT(3, 2) << "  |\n"
       << "'. "  << MAT(0, 3) << MAT(1, 3) << MAT(2, 3) << MAT(3, 3) << " .'\n";

#undef MAT

    os.precision(oldPrecision);
    return os;
}


#ifndef UTILS_ANGLES_H
#define UTILS_ANGLES_H

#include <glm/glm.hpp>

namespace sb
{
    namespace Math
    {
        template<typename Float> class Radians;

        template<typename Float>
        class Degrees
        {
        public:
            explicit Degrees(Float value): mDegrees(value) {}

            Degrees(const Radians<Float>& radians):
                mDegrees(glm::degrees(radians.value()))
            {}

            inline Float value() const { return mDegrees; }

        private:
            Float mDegrees;
        };

        template<typename Float>
        class Radians
        {
        public:
            explicit Radians(Float value): mRadians(value) {}

            Radians(const Degrees<Float>& degrees):
                mRadians(glm::radians(degrees.value()))
            {}

            inline Float value() const { return mRadians; }
        private:
            Float mRadians;
        };

        template<typename Float>
        std::ostream& operator <<(std::ostream& os, const Degrees<Float>& deg)
        {
            return os << deg.value() << " deg";
        }

        template<typename Float>
        std::ostream& operator <<(std::ostream& os, const Radians<Float>& rad)
        {
            return os << rad.value() << " rad";
        }
    } // namespace Math
} // namespace sb

#endif /* UTILS_ANGLES_H */

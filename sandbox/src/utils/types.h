#ifndef UTILS_TYPES_H
#define UTILS_TYPES_H

#include <ostream>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

typedef glm::float4x4 Mat44f;
typedef glm::float3x3 Mat33f;
typedef glm::float4 Vec4f;
typedef glm::float3 Vec3f;
typedef glm::float2 Vec2f;
typedef glm::fquat Quatf;

typedef glm::int2 Vec2i;
typedef glm::double3 Vec3d;

typedef Mat44f Mat44;
typedef Mat33f Mat33;
typedef Vec4f Vec4;
typedef Vec3f Vec3;
typedef Vec2f Vec2;
typedef Quatf Quat;


#define Z_NEAR 0.01f
#define Z_FAR 100000.f

#define SAFE_RELEASE(ptr) ((delete (ptr)), (ptr) = NULL)

#define PI glm::pi<float>()
#define PI2 (glm::pi<float>() * 2.f)
#define PI_2 (glm::half_pi<float>())
#define PI_3 (glm::pi<float>() * 0.33333333f)

template<typename T>
std::ostream& operator <<(std::ostream& os,
                          const glm::detail::tvec3<T, glm::highp>& v)
{
    return os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
}

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

typedef sb::Math::Degrees<float> Degrees;
typedef sb::Math::Radians<float> Radians;

inline Degrees operator "" _deg(long double degrees)
{
    return Degrees(degrees);
}

inline Radians operator "" _rad(long double radians)
{
    return Radians(radians);
}

#endif // UTILS_TYPES_H


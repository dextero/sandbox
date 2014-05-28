#ifndef UTILS_TYPES_H
#define UTILS_TYPES_H

#include <ostream>
#include <utility>
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "utils/vector.h"
#include "utils/angles.h"

typedef glm::float4x4 Mat44f;
typedef glm::float3x3 Mat33f;
typedef glm::fquat Quatf;

typedef sb::TVec2<int> Vec2i;
typedef sb::TVec2<float> Vec2f;

typedef sb::TVec3<float> Vec3f;
typedef sb::TVec3<double> Vec3d;

typedef sb::TVec4<float> Vec4f;

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

template<typename T>
std::ostream& operator <<(std::ostream& os,
                          const glm::detail::tvec3<T, glm::highp>& v)
{
    return os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
}

#endif // UTILS_TYPES_H


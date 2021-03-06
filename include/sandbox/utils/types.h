#ifndef UTILS_TYPES_H
#define UTILS_TYPES_H

#include <ostream>
#include <utility>
#include <functional>

#ifndef GLM_FORCE_RADIANS
#   define GLM_FORCE_RADIANS
#endif
#ifndef GLM_SWIZZLE
#   define GLM_SWIZZLE
#endif
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <sandbox/utils/vector.h>
#include <sandbox/utils/angles.h>

namespace sb {

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

} // namespace sb

inline sb::Degrees operator "" _deg(long double degrees)
{
    return sb::Degrees(degrees);
}

inline sb::Radians operator "" _rad(long double radians)
{
    return sb::Radians(radians);
}

#endif // UTILS_TYPES_H


#ifndef UTILS_TYPES_H
#define UTILS_TYPES_H

typedef unsigned int uint;

#include <cml/cml.h>

typedef cml::matrix44f_c	Mat44f;
typedef cml::matrix33f_c	Mat33f;
typedef cml::vector4f		Vec4f;
typedef cml::vector3f		Vec3f;
typedef cml::vector2f		Vec2f;
typedef cml::quaternionf	Quatf;

typedef cml::vector2i		Vec2i;
typedef cml::vector3d		Vec3d;

typedef Mat44f				Mat44;
typedef Mat33f				Mat33;
typedef Vec4f				Vec4;
typedef Vec3f				Vec3;
typedef Vec2f				Vec2;
typedef Quatf				Quat;


#define Z_NEAR	0.01f
#define Z_FAR	100000.f

#define SAFE_RELEASE(ptr)	(delete (ptr)), (ptr) = NULL

#define PI		cml::constantsf::pi()
#define PI2		(cml::constantsf::pi() * 2.f)
#define PI_2	(cml::constantsf::pi_over_2())
#define PI_3	(cml::constantsf::pi() * 0.33333333f)

#endif // UTILS_TYPES_H
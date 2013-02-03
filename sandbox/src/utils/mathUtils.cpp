#include "mathUtils.h"

namespace sb
{
	namespace Math
	{
		Mat44 MatrixOrthographic(float left, float right, float bottom, float top, float near, float far)
		{
			Mat44 ret;
			cml::matrix_orthographic_RH(ret, left, right, bottom, top, near, far, cml::z_clip_neg_one);
			return ret;
		}
	
		Mat44 MatrixPerspective(float fov, float aspectRatio, float near, float far)
		{
			Mat44 ret;
			cml::matrix_perspective_xfov(ret, fov, aspectRatio, near, far, cml::right_handed, cml::z_clip_neg_one);
			return ret;
		}
	} // namespace Math
} // namespace sb

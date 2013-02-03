#include "light.h"
#include "../utils/profiler.h"

namespace sb
{
	Light::Light(ELightType type, Color color, Vec3 posOrDir, float intensity):
		mType(type),
		mColor(color),
		mPos(posOrDir),
		mIntensity(intensity)
	{
		PROFILE();
	}
} // namespace sb

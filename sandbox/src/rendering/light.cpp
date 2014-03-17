#include "light.h"
#include "../utils/profiler.h"

namespace sb
{
    Light::Light(ELightType type, Color color, Vec3 posOrDir, float intensity):
        mPos(posOrDir),
        mColor(color),
        mIntensity(intensity),
        mType(type)
    {
        PROFILE();
    }
} // namespace sb

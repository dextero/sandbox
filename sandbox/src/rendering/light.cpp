#include "light.h"

namespace sb
{
    Light::Light(ELightType type, Color color, Vec3 posOrDir, float intensity):
        mPos(posOrDir),
        mColor(color),
        mIntensity(intensity),
        mType(type)
    {}
} // namespace sb

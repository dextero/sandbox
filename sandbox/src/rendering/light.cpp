#include "light.h"

namespace sb
{
    Light::Light(Type type,
                 const Vec3& posOrDir,
                 float intensity,
                 const Color& color):
        type(type),
        pos(posOrDir),
        intensity(intensity),
        color(color)
    {}
} // namespace sb

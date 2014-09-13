#include "light.h"

namespace sb {
namespace {
    const unsigned SHADOWMAP_WIDTH = 4 * 1024;
    const unsigned SHADOWMAP_HEIGHT = SHADOWMAP_WIDTH;
} // namespace

Light::Light(Type type,
             const Vec3& posOrDir,
             float intensity,
             const Color& color,
             bool makesShadows):
    type(type),
    pos(posOrDir),
    intensity(intensity),
    color(color),
    makesShadows(makesShadows),
    shadowFramebuffer(std::make_shared<Framebuffer>(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT))
{}

} // namespace sb

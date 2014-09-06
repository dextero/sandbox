#ifndef LIGHT_H
#define LIGHT_H

#include "color.h"
#include "utils/types.h"
#include "utils/debug.h"
#include "rendering/framebuffer.h" 

namespace sb
{
    struct Light
    {
    public:
        enum class Type {
            Point,
            Parallel
        } type;

        Vec3 pos;
        float intensity; 
        Color color;
        const bool makesShadows;

        static Light point(const Vec3& pos,
                           float intensity,
                           const Color& color = Color::White,
                           bool makesShadows = false) {
            return Light(Type::Point, pos, intensity, color, makesShadows);
        }

        static Light parallel(const Vec3& dir,
                              float intensity,
                              const Color& color = Color::White,
                              bool makesShadows = true) {
            return Light(Type::Parallel, dir, intensity, color, makesShadows);
        }

        std::shared_ptr<const Texture> getShadowMap() const {
            sbAssert(makesShadows, "makesShadows must be set to true to use shadow maps");
            return shadowFramebuffer->getTexture();
        }

    private:
        std::shared_ptr<Framebuffer> shadowFramebuffer;

        Light(Type type,
              const Vec3& posOrDir,
              float intensity,
              const Color& color,
              bool makesShadows);

        friend class Renderer;
    };
} // namespace sb

#endif // LIGHT_H

#ifndef LIGHT_H
#define LIGHT_H

#include "color.h"
#include "../utils/types.h"

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

        static Light point(const Vec3& pos,
                           float intensity,
                           const Color& color) {
            return Light(Type::Point, pos, intensity, color);
        }

        static Light parallel(const Vec3& dir,
                              float intensity,
                              const Color& color = Color::White) {
            return Light(Type::Parallel, dir, intensity, color);
        }

    private:
        Light(Type type,
              const Vec3& posOrDir,
              float intensity,
              const Color& color);
    };
} // namespace sb

#endif // LIGHT_H

#ifndef TERRAIN_H
#define TERRAIN_H

#include "drawable.h"

namespace sb
{
    class Terrain: public Drawable
    {
    public:
        Terrain(const std::string& heightmap,
                const std::string& texture,
                const std::shared_ptr<Shader>& shader);

        float getHeightAt(Vec2 xz) const { return getHeightAt(xz.x, xz.y); }
        float getHeightAt(float x, float z) const;

        static inline float decodeHeight(uint32_t rgba)
        {
            return (float)(((rgba) & 0x00ff0000)
                           | (((rgba) & 0xff000000) << 8)
                           | (((rgba) & 0x0000ff00) << 16))
                        / (float)0xffffff00;
        }

    private:
        std::shared_ptr<Image> mImg;
    };
} // namespace sb

#endif //TERRAIN_H

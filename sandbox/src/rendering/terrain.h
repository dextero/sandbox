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
    };
} // namespace sb

#endif //TERRAIN_H

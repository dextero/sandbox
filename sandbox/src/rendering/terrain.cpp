#include "terrain.h"
#include "../resources/resourceMgr.h"

namespace sb
{
    Terrain::Terrain(const std::string& heightmap,
                     const std::string& texture,
                     const std::shared_ptr<Shader>& shader):
        Drawable(ProjectionPerspective,
                 gResourceMgr.getTerrain(heightmap),
                 gResourceMgr.getTexture(texture),
                 shader)
    {}
} // namespace sb

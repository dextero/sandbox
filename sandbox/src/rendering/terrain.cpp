#include "terrain.h"
#include "../resources/resourceMgr.h"

namespace sb
{
    Terrain::Terrain(const std::string& heightmap,
                     const std::string& texture):
        Drawable(ProjectionPerspective)
    {
        mMesh = gResourceMgr.getTerrain(heightmap);
        mTexture = gResourceMgr.getTexture(texture);
    }
} // namespace sb

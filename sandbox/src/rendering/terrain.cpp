#include "terrain.h"
#include "../resources/resourceMgr.h"

namespace sb
{
    Terrain::Terrain(const std::string& heightmap,
                     const std::string& texture):
        Drawable(ProjectionPerspective)
    {
        mMesh = gResourceMgr.GetTerrain(heightmap);
        mTexture = gResourceMgr.GetTexture(texture);
    }
} // namespace sb

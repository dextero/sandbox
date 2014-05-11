#include "terrain.h"
#include "../resources/resourceMgr.h"

namespace sb
{
    Terrain::Terrain(const std::wstring& heightmap, const std::wstring& texture):
        Drawable(ProjectionPerspective)
    {
        mMesh = gResourceMgr.GetTerrain(heightmap);
        mTexture = gResourceMgr.GetTexture(texture);
    }
} // namespace sb

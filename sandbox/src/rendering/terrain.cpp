#include "terrain.h"
#include "../utils/profiler.h"
#include "../resources/resourceMgr.h"

namespace sb
{
	Terrain::Terrain(const std::wstring& heightmap, const std::wstring& texture):
		Drawable(ProjectionPerspective)
	{
		PROFILE();
	
		mMesh = gResourceMgr.GetTerrain(heightmap);
		mTexture = gResourceMgr.GetTexture(texture);
	}
} // namespace sb

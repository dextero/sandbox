#ifndef TERRAIN_H
#define TERRAIN_H

#include "drawable.h"

namespace sb
{
	class Terrain: public Drawable
	{
	public:
		Terrain(const std::wstring& heightmap, const std::wstring& texture);
	};
} // namespace sb
	
#endif //TERRAIN_H
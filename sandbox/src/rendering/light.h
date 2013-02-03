#ifndef LIGHT_H
#define LIGHT_H

#include "color.h"
#include "../utils/types.h"

namespace sb
{
	class Light
	{
	public:
		Vec3 mPos;
		Color mColor;
		float mIntensity;
	
		enum ELightType {
			LightAmbient,
			LightPoint,
			LightParallel
		} mType;
	
		Light(ELightType type, Color color, Vec3 posOrDir, float intensity);
	};
} // namespace sb
	
#endif // LIGHT_H
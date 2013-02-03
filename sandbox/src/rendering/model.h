#ifndef MODEL_H
#define MODEL_H

#include "drawable.h"
#include <string>

namespace sb
{
	class Model: public Drawable
	{
	public:
		Model();
		Model(const std::wstring& path);
	
		bool LoadFromFile(const std::wstring& path);
	};
} // namespace sb
	
#endif //MODEL_H
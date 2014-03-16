#ifndef SPRITE_H
#define SPRITE_H

#include "drawable.h"
#include <string>

namespace sb
{
    class Sprite: public Drawable
    {
    public:
        Sprite();
        Sprite(const std::wstring& image);

        void SetImage(const std::wstring& image);
    };
} // namespace sb

#endif //SPRITE_H

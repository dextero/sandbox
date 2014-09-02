#ifndef SPRITE_H
#define SPRITE_H

#include "drawable.h"
#include <string>

namespace sb
{
    class Sprite: public Drawable
    {
    public:
        Sprite(const std::shared_ptr<Shader>& shader);
        Sprite(const std::string& image,
               const std::shared_ptr<Shader>& shader);

        void setImage(const std::string& image);
        void setTexture(const std::shared_ptr<const Texture>& tex);
    };
} // namespace sb

#endif //SPRITE_H

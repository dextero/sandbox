#ifndef SRC_RENDERER_TEXTURE_H
#define SRC_RENDERER_TEXTURE_H

#include <memory>

#include "rendering/types.h"
#include "resources/image.h"

namespace sb
{
    class Texture
    {
    public:
        static const uint32_t MAX_TEXTURE_UNITS = 4;

        Texture(std::shared_ptr<Image> image);
        ~Texture();

        Texture(Texture&&) = delete;
        Texture& operator =(Texture&&) = delete;
        Texture(const Texture&) = delete;
        Texture& operator =(const Texture&) = delete;

        void bind(uint32_t textureUnit) const;
        void unbind() const;

    private:
        TextureId mId;
    };
} // namespace sb

#endif /* SRC_RENDERER_TEXTURE_H */

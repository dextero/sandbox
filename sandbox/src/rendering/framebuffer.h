#ifndef RENDERING_FRAMEBUFFER_H
#define RENDERING_FRAMEBUFFER_H

#include <memory>

#include "rendering/types.h"
#include "rendering/texture.h"

#define WITH_RENDERBUFFER 1

namespace sb {

class Framebuffer
{
public:
    Framebuffer(uint32_t width,
                uint32_t height);

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator =(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& old) = delete;
    Framebuffer& operator =(Framebuffer&& old) = delete;

    virtual ~Framebuffer();

    void bind() const;
    void unbind() const;

    BufferId getId() const { return id; }
    std::shared_ptr<const Texture> getTexture() const
    {
        return texture;
    }

    const Vec2i& getSize() const { return sizePixels; }

private:
    Vec2i sizePixels;
    BufferId id;
#if WITH_RENDERBUFFER
    BufferId renderbufferId;
#endif
    std::shared_ptr<Texture> texture;
};

} // namespace sb

#endif /* RENDERING_FRAMEBUFFER_H */

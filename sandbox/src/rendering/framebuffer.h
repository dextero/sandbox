#ifndef RENDERING_FRAMEBUFFER_H
#define RENDERING_FRAMEBUFFER_H

#include <memory>

#include "rendering/types.h"
#include "rendering/texture.h"

namespace sb {

class Framebuffer
{
public:
    Framebuffer(unsigned width,
                unsigned height);

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

private:
    BufferId id;
    std::shared_ptr<Texture> texture;
};

} // namespace sb

#endif /* RENDERING_FRAMEBUFFER_H */

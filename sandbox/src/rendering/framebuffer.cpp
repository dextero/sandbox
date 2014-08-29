#include "rendering/framebuffer.h"

#include "utils/lib.h"
#include "utils/misc.h"

namespace sb {

Framebuffer::Framebuffer(unsigned width,
                         unsigned height):
    id(0),
    texture(std::make_shared<Texture>(width, height))
{
    gLog.debug("new framebuffer");
    GL_CHECK(glGenFramebuffers(1, &id));

    auto bind = make_bind(*this);

    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                    GL_TEXTURE_2D, texture->getId(), 0));
}

Framebuffer::~Framebuffer()
{
    if (id) {
        GL_CHECK(glDeleteFramebuffers(1, &id));
        id = 0;
    }
}

void Framebuffer::bind() const
{
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));
}

void Framebuffer::unbind() const
{
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

} // namespace sb


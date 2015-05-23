#include <sandbox/rendering/framebuffer.h>

#include <sandbox/utils/lib.h>
#include <sandbox/utils/misc.h>
#include <sandbox/utils/debug.h>

namespace sb {

Framebuffer::Framebuffer(uint32_t width,
                         uint32_t height):
    sizePixels(width, height),
    id(0),
#if WITH_RENDERBUFFER
    renderbufferId(0),
#endif
    texture(std::make_shared<Texture>(width, height))
{
    GL_CHECK(glGenFramebuffers(1, &id));
    auto bind = make_bind(*this);

#if WITH_RENDERBUFFER
    GL_CHECK(glGenRenderbuffers(1, &renderbufferId));
    GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, renderbufferId));
    GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                                   width, height));
    GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, 0));
#endif

    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                    GL_TEXTURE_2D, texture->getId(), 0));
    GL_CHECK(glDrawBuffer(GL_NONE));
    GL_CHECK(glReadBuffer(GL_NONE));

#if WITH_RENDERBUFFER
    //GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                       //GL_RENDERBUFFER, renderbufferId));
#endif

    GLenum fboStatus;
    GL_CHECK(fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        sbFail("framebuffer not ready");
    }
}

Framebuffer::~Framebuffer()
{
#if WITH_RENDERBUFFER
    if (renderbufferId) {
        GL_CHECK(glDeleteRenderbuffers(1, &renderbufferId));
        renderbufferId = 0;
    }
#endif
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
    //GL_CHECK(glDrawBuffer(GL_BACK));
    //GL_CHECK(glReadBuffer(GL_BACK));

    auto texBind = make_bind(*texture, 0);
    GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
}

} // namespace sb


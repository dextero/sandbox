#include "rendering/texture.h"

#include <vector>

#include "utils/logger.h"
#include "utils/lib.h"
#include "utils/math.h"
#include "utils/misc.h"

namespace sb {
namespace {

GLuint createTexture(unsigned width,
                     unsigned height,
                     void* data,
                     ILuint imageFormat,
                     ILuint imageType,
                     bool generateMipmaps)
{
    GLuint id = 0;

    // copy image to opengl
    GL_CHECK(glGenTextures(1, &id));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

    GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    if (imageFormat == GL_DEPTH_COMPONENT) {
        GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS));
        GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    }

    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, imageFormat,
                          width, height, 0, imageFormat,
                          imageType, data));

    if (generateMipmaps) {
        GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
    }

    return id;
}

} // namespace

Texture::Texture(unsigned width,
                 unsigned height):
    mId(createTexture(width, height, nullptr, GL_DEPTH_COMPONENT, GL_FLOAT, true))
{
}

Texture::Texture(std::shared_ptr<Image> image):
    mId(0)
{
    uint32_t maxTexSize;
    GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&maxTexSize));

    uint32_t imgWidth = image->getWidth();
    uint32_t imgHeight = image->getHeight();

#if NO_NON_POT_TEXTURES
    uint32_t potWidth = math::nextPowerOf2(imgWidth);
    uint32_t potHeight = math::nextPowerOf2(imgHeight);

    if (imgWidth != potWidth || imgHeight != potHeight) {
        auto copy = std::make_shared<Image>(*image);
        copy->scale(potWidth, potHeight);
        image = copy;
        imgWidth = potWidth;
        imgHeight = potHeight;
    }
#endif

    GLuint prevTex;
    // save current image
    GL_CHECK(glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&prevTex));

    mId = createTexture(imgWidth, imgHeight, ilGetData(),
                        ilGetInteger(IL_IMAGE_FORMAT),
                        ilGetInteger(IL_IMAGE_TYPE), true);

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, prevTex));
}

Texture::~Texture()
{
    if (mId) {
        glDeleteTextures(1, &mId);
    }
}

void Texture::bind(uint32_t textureUnit) const
{
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + textureUnit));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, mId));
}

void Texture::unbind() const
{
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

void Texture::setMagFilter(MagFilter filter) const
{
    GLuint magFilter = filter == MagFilter::Nearest ? GL_NEAREST : GL_LINEAR;

    auto bind = make_bind(*this, 0);
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter));
}

} // namespace sb


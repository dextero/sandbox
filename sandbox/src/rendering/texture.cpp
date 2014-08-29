#include "rendering/texture.h"

#include "utils/logger.h"
#include "utils/lib.h"
#include "utils/math.h"

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

    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, imageFormat,
                          width, height, 0, imageFormat,
                          imageType, data));

    // generate mipmaps
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    if (generateMipmaps) {
        GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
    }

    return id;
}

} // namespace

Texture::Texture(unsigned width,
                 unsigned height):
    mId(createTexture(width, height, nullptr, GL_DEPTH_COMPONENT, GL_FLOAT, false))
{
}

Texture::Texture(std::shared_ptr<Image> image):
    mId(0)
{
    uint32_t maxTexSize;
    GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&maxTexSize));

    uint32_t imgWidth = image->getWidth();
    uint32_t imgHeight = image->getHeight();
    uint32_t potWidth = math::nextPowerOf2(imgWidth);
    uint32_t potHeight = math::nextPowerOf2(imgHeight);

    if (imgWidth != potWidth || imgHeight != potHeight) {
        auto copy = std::make_shared<Image>(*image);
        copy->scale(potWidth, potHeight);
        image = copy;
    }

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

} // namespace sb


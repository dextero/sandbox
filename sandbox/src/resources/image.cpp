#include "image.h"
#include "utils/lib.h"
#include "utils/logger.h"
#include "utils/stringUtils.h"

#include <IL/ilu.h>

namespace sb
{
    Image::Image(): mId(0u) {}

    Image::Image(const std::string& file):
        mId(0u)
    {
        if (!loadFromFile(file)) {
            gLog.err("couldn't load image %s\n", file.c_str());
        }
    }

    Image::~Image()
    {
        IL_CHECK(ilBindImage(mId));
        IL_CHECK(ilDeleteImage(mId));
    }

    Image::Image(const Image& copy):
        mId(0)
    {
        *this = copy;
    }

    Image::Image(Image&& source):
        mId(0)
    {
        *this = std::forward<Image>(source);
    }

    Image& Image::operator =(const Image& copy)
    {
        IL_CHECK(mId = ilGenImage());
        IL_CHECK(ilBindImage(mId));
        IL_CHECK(ilCopyImage(copy.mId));
        return *this;
    }

    Image& Image::operator =(Image&& source)
    {
        mId = source.mId;
        source.mId = 0;
        return *this;
    }

    bool Image::loadFromFile(const std::string& file)
    {
        gLog.info("loading image %s\n", file.c_str());

        IL_CHECK(mId = ilGenImage());
        IL_CHECK_RET(ilBindImage(mId), false);

#ifdef PLATFORM_WIN32
        IL_CHECK_RET(ilLoadImage(utils::toWString(file).c_str()), false);
#else //PLATFORM_LINUX
        IL_CHECK_RET(ilLoadImage(file.c_str()), false);
#endif // PLATFORM_WIN32

        return true;
    }

    uint32_t Image::getWidth()
    {
        IL_CHECK(ilBindImage(mId));
        return ilGetInteger(IL_IMAGE_WIDTH);
    }

    uint32_t Image::getHeight()
    {
        IL_CHECK(ilBindImage(mId));
        return ilGetInteger(IL_IMAGE_HEIGHT);
    }

    void Image::scale(uint32_t newWidth,
                      uint32_t newHeight)
    {
        IL_CHECK(ilBindImage(mId));

        uint32_t width = ilGetInteger(IL_IMAGE_WIDTH);
        uint32_t height = ilGetInteger(IL_IMAGE_HEIGHT);

        if (width == newWidth && height == newHeight) {
            return;
        }

        gLog.trace("scaling texture: %ux%u to %ux%u\n",
                   width, height, newWidth, newHeight);

        iluScale(newWidth, newHeight, 1);
    }

    void* Image::getRGBAData()
    {
        IL_CHECK(ilBindImage(mId));

        if (ilGetInteger(IL_IMAGE_FORMAT) != IL_RGBA) {
            IL_CHECK_RET(ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE), nullptr);
        }

        return ilGetData();
    }
} // namespace sb


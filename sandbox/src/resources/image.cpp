#include "image.h"
#include "utils/lib.h"
#include "utils/logger.h"
#include "utils/stringUtils.h"


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

        IL_CHECK_RET(ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE), false);

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

    void* Image::getData()
    {
        IL_CHECK(ilBindImage(mId));
        return ilGetData();
    }
} // namespace sb


#include "image.h"
#include "utils/profiler.h"
#include "utils/libUtils.h"
#include "utils/logger.h"
#include "utils/stringUtils.h"


namespace sb
{
    Image::Image(): mId(0u)
    {
        PROFILE();
    }

    Image::Image(const std::wstring& file):
        mId(0u)
    {
        PROFILE();

        if (!LoadFromFile(file))
            gLog.Err("couldn't load image %ls\n", file.c_str());
    }

    bool Image::LoadFromFile(const std::wstring& file)
    {
        PROFILE();

        IL_CHECK(mId = ilGenImage());
        IL_CHECK_RET(ilBindImage(mId), false);

#ifdef PLATFORM_WIN32
        IL_CHECK_RET(ilLoadImage(file.c_str()), false);
#else //PLATFORM_LINUX
        IL_CHECK_RET(ilLoadImage(StringUtils::ToString(file).c_str()), false);
#endif // PLATFORM_WIN32

        IL_CHECK_RET(ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE), false);

        return true;
    }

    uint32_t Image::GetWidth()
    {
        PROFILE();

        IL_CHECK(ilBindImage(mId));
        return ilGetInteger(IL_IMAGE_WIDTH);
    }

    uint32_t Image::GetHeight()
    {
        PROFILE();

        IL_CHECK(ilBindImage(mId));
        return ilGetInteger(IL_IMAGE_HEIGHT);
    }

    void* Image::GetData()
    {
        PROFILE();

        IL_CHECK(ilBindImage(mId));
        return ilGetData();
    }
} // namespace sb


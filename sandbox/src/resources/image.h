#ifndef IMAGE_H
#define IMAGE_H

#include <IL/il.h>
#include "rendering/types.h"
#include "utils/types.h"
#include <string>

namespace sb
{
    class Image
    {
        ILuint mId;
    public:
        Image();
        Image(const std::wstring& file);

        bool LoadFromFile(const std::wstring& file);

        uint GetWidth();
        uint GetHeight();
        void* GetData();
    };
} // namespace sb

#endif //IMAGE_H

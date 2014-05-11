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
        Image(const std::string& file);

        bool LoadFromFile(const std::string& file);

        uint32_t GetWidth();
        uint32_t GetHeight();
        void* GetData();
    };
} // namespace sb

#endif //IMAGE_H

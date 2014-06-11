#ifndef IMAGE_H
#define IMAGE_H

#include <string>

#include <IL/il.h>

#include "rendering/types.h"
#include "utils/types.h"

namespace sb
{
    class Image
    {
    public:
        Image();
        Image(const std::string& file);

        ~Image();
        Image(const Image& copy);
        Image(Image&& source);
        Image& operator =(const Image& copy);
        Image& operator =(Image&& source);

        bool loadFromFile(const std::string& file);

        uint32_t getWidth();
        uint32_t getHeight();

        void scale(uint32_t newWidth,
                   uint32_t newHeight);

        void* getRGBAData();

    private:
        ILuint mId;
    };
} // namespace sb

#endif //IMAGE_H

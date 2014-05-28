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
    public:
        Image();
        Image(const std::string& file);

        bool loadFromFile(const std::string& file);

        uint32_t getWidth();
        uint32_t getHeight();
        void* getData();

    private:
        ILuint mId;
    };
} // namespace sb

#endif //IMAGE_H

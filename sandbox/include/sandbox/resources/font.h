#ifndef RESOURCES_FONT_H
#define RESOURCES_FONT_H

#include <array>

#include <sandbox/rendering/texture.h>
#include <sandbox/utils/rect.h>
#include <sandbox/utils/lexical_cast.h>

namespace sb {

class Font
{
public:
    struct Letter
    {
        FloatRect texcoords;
        uint32_t widthPixels;
        uint32_t heightPixels;

        Letter():
            widthPixels(0),
            heightPixels(0)
        {}

        bool isInitialized() const
        {
            return !texcoords.isEmpty()
                    && widthPixels > 0
                    && heightPixels > 0;
        }
    };

    Font(const std::shared_ptr<Texture>& texture,
         const std::array<Letter, 256>& letters):
        mTexture(texture),
        mLetters(letters),
        lineHeightPixels(0)
    {
        for (const Letter& l: letters) {
            if (l.heightPixels > lineHeightPixels) {
                lineHeightPixels = l.heightPixels;
            }
        }
    }

    uint32_t getLineHeightPixels() const { return lineHeightPixels; }
    const Letter& getLetter(uint8_t idx) const { return mLetters[idx]; }
    const std::shared_ptr<Texture>& getTexture() const { return mTexture; }

private:
    const std::shared_ptr<Texture> mTexture;
    const std::array<Letter, 256> mLetters;
    uint32_t lineHeightPixels;

    friend class ResourceMgr;
};

} // namespace sb

#endif // RESOURCES_FONT_H

#ifndef RENDERING_TEXT_H
#define RENDERING_TEXT_H

#include <string>
#include <memory>

#include <sandbox/rendering/drawable.h>

namespace sb {

class Text: public Drawable
{
public:
    Text(const std::string& text,
         const std::shared_ptr<Font>& font,
         const std::shared_ptr<Shader>& shader);

private:
    std::shared_ptr<Font> mFont;
    std::string mText;
};

} // namespace sb

#endif // RENDERING_TEXT_H

#include "sprite.h"
#include "../resources/resourceMgr.h"

namespace sb
{
    Sprite::Sprite(const std::shared_ptr<Shader>& shader):
        Drawable(ProjectionOrthographic,
                 gResourceMgr.getQuad(),
                 gResourceMgr.getTexture("default.png"),
                 shader)
    {}

    Sprite::Sprite(const std::string& image,
                   const std::shared_ptr<Shader>& shader):
        Drawable(ProjectionOrthographic,
                 gResourceMgr.getQuad(),
                 gResourceMgr.getTexture(image),
                 shader)
    {}

    void Sprite::setImage(const std::string& image)
    {
        mTexture = gResourceMgr.getTexture(image);
    }
} // namespace sb

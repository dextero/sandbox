#include "sprite.h"
#include "../resources/resourceMgr.h"

namespace sb
{
    Sprite::Sprite(const std::shared_ptr<Shader>& shader):
        Drawable(ProjectionType::Orthographic,
                 gResourceMgr.getQuad(),
                 gResourceMgr.getTexture("default.png"),
                 shader)
    {}

    Sprite::Sprite(const std::string& image,
                   const std::shared_ptr<Shader>& shader):
        Drawable(ProjectionType::Orthographic,
                 gResourceMgr.getQuad(),
                 gResourceMgr.getTexture(image),
                 shader)
    {}

    void Sprite::setImage(const std::string& image)
    {
        mTexture = gResourceMgr.getTexture(image);
    }

    void Sprite::setTexture(const std::shared_ptr<const Texture>& tex)
    {
        mTexture = tex;
    }
} // namespace sb

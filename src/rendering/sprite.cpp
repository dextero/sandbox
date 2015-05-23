#include <sandbox/rendering/sprite.h>
#include <sandbox/resources/resourceMgr.h>

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
        setTexture(gResourceMgr.getTexture(image));
    }
} // namespace sb

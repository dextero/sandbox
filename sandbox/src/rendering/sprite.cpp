#include "sprite.h"
#include "../resources/resourceMgr.h"

namespace sb
{
    Sprite::Sprite():
        Drawable(ProjectionOrthographic)
    {
        mTexture = gResourceMgr.getTexture("default.png");
        mMesh = NULL;
    }

    Sprite::Sprite(const std::string& image):
        Drawable(ProjectionOrthographic)
    {
        mTexture = gResourceMgr.getTexture(image);
        mMesh = NULL;
    }

    void Sprite::SetImage(const std::string& image)
    {
        mTexture = gResourceMgr.getTexture(image);
    }
} // namespace sb

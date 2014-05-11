#include "sprite.h"
#include "../resources/resourceMgr.h"

namespace sb
{
    Sprite::Sprite():
        Drawable(ProjectionOrthographic)
    {
        mTexture = gResourceMgr.GetTexture("default.png");
        mMesh = NULL;
    }

    Sprite::Sprite(const std::string& image):
        Drawable(ProjectionOrthographic)
    {
        mTexture = gResourceMgr.GetTexture(image);
        mMesh = NULL;
    }

    void Sprite::SetImage(const std::string& image)
    {
        gResourceMgr.FreeTexture(mTexture);
        mTexture = gResourceMgr.GetTexture(image);
    }
} // namespace sb

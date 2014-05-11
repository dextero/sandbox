#include "sprite.h"
#include "../resources/resourceMgr.h"

namespace sb
{
    Sprite::Sprite():
        Drawable(ProjectionOrthographic)
    {
        mTexture = gResourceMgr.GetTexture(L"default.png");
        mMesh = NULL;
    }

    Sprite::Sprite(const std::wstring& image):
        Drawable(ProjectionOrthographic)
    {
        mTexture = gResourceMgr.GetTexture(image);
        mMesh = NULL;
    }

    void Sprite::SetImage(const std::wstring& image)
    {
        gResourceMgr.FreeTexture(mTexture);
        mTexture = gResourceMgr.GetTexture(image);
    }
} // namespace sb

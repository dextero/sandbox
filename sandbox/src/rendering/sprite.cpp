#include "sprite.h"
#include "../resources/resourceMgr.h"
#include "../utils/profiler.h"

namespace sb
{
    Sprite::Sprite():
        Drawable(ProjectionOrthographic)
    {
        PROFILE();

        mTexture = gResourceMgr.GetTexture(L"default.png");
        mMesh = NULL;
    }

    Sprite::Sprite(const std::wstring& image):
        Drawable(ProjectionOrthographic)
    {
        PROFILE();

        mTexture = gResourceMgr.GetTexture(image);
        mMesh = NULL;
    }

    void Sprite::SetImage(const std::wstring& image)
    {
        gResourceMgr.FreeTexture(mTexture);
        mTexture = gResourceMgr.GetTexture(image);
    }
} // namespace sb

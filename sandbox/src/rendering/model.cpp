#include "model.h"
#include "../utils/profiler.h"

namespace sb
{
    Model::Model():
        Drawable(ProjectionPerspective)
    {}

    Model::Model(const std::wstring& path):
        Drawable(ProjectionPerspective)
    {
        mMesh = gResourceMgr.GetMesh(path);
    }

    bool Model::LoadFromFile(const std::wstring& path)
    {
        mMesh = gResourceMgr.GetMesh(path);
        return mMesh != NULL;
    }
}

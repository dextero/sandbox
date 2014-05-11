#include "model.h"

namespace sb
{
    Model::Model():
        Drawable(ProjectionPerspective)
    {}

    Model::Model(const std::string& path):
        Drawable(ProjectionPerspective)
    {
        mMesh = gResourceMgr.GetMesh(path);
    }

    bool Model::LoadFromFile(const std::string& path)
    {
        mMesh = gResourceMgr.GetMesh(path);
        return mMesh != NULL;
    }
}

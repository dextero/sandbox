#include "model.h"

namespace sb
{
    Model::Model():
        Drawable(ProjectionPerspective)
    {}

    Model::Model(const std::string& path):
        Drawable(ProjectionPerspective)
    {
        mMesh = gResourceMgr.getMesh(path);
    }

    bool Model::LoadFromFile(const std::string& path)
    {
        mMesh = gResourceMgr.getMesh(path);
        return mMesh != NULL;
    }
}

#include "model.h"

namespace sb
{
    Model::Model(const std::string& path,
                 const std::shared_ptr<Shader>& shader):
        Drawable(ProjectionPerspective,
                 gResourceMgr.getMesh(path),
                 nullptr,
                 shader)
    {}

    bool Model::loadFromFile(const std::string& path)
    {
        mMesh = gResourceMgr.getMesh(path);
        return mMesh != NULL;
    }
}

#include "model.h"

namespace sb
{
    Model::Model(const std::string& path,
                 const std::shared_ptr<Shader>& shader,
                 const std::shared_ptr<Texture>& texture):
        Drawable(ProjectionType::Perspective,
                 gResourceMgr.getMesh(path),
                 texture,
                 shader)
    {}

    bool Model::loadFromFile(const std::string& path)
    {
        mMesh = gResourceMgr.getMesh(path);
        return mMesh != NULL;
    }
}

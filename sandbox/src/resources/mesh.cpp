#include "mesh.h"

#include "utils/lib.h"
#include "utils/logger.h"
#include "resources/resourceMgr.h"

namespace sb
{
    Mesh::Mesh(EShape shape,
               const std::vector<Vec3>& vertices,
               const std::vector<Vec2>& texcoords,
               const std::vector<Color>& colors,
               const std::vector<uint32_t>& indices,
               std::shared_ptr<Texture> texture):
        mVertexBuffer(vertices, texcoords, colors),
        mIndexBuffer(indices),
        mIndexBufferSize(indices.size()),
        mShape(shape),
        mTexture(texture)
    {}
} // namespace sb

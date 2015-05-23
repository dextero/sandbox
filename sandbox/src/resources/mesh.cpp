#include <sandbox/resources/mesh.h>

#include <sandbox/utils/lib.h>
#include <sandbox/utils/logger.h>
#include <sandbox/resources/resourceMgr.h>

namespace sb
{
    Mesh::Mesh(Shape shape,
               const std::vector<Vec3>& vertices,
               const std::vector<Vec2>& texcoords,
               const std::vector<Color>& colors,
               const std::vector<Vec3>& normals,
               const std::vector<uint32_t>& indices,
               std::shared_ptr<Texture> texture):
        mVertexBuffer(vertices, texcoords, colors, normals),
        mIndexBuffer(indices),
        mIndexBufferSize(indices.size()),
        mShape(shape),
        mTexture(texture)
    {}
} // namespace sb

#include "mesh.h"

#include "utils/libUtils.h"
#include "utils/logger.h"
#include "resources/resourceMgr.h"

namespace sb
{
    Mesh::Mesh(EShape shape,
               const std::vector<Vec3>& vertices,
               const std::vector<Vec2>& texcoords,
               const std::vector<Color>& colors,
               const std::vector<uint32_t>& indices,
               std::shared_ptr<TextureId> texture):
        mVertexBuffer(vertices, texcoords, colors),
        mIndexBuffer(0),
        mIndexBufferSize(0u),
        mShape(shape),
        mTexture(texture)
    {
        GL_CHECK(glGenBuffers(1, &mIndexBuffer));
        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer));
        GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                              indices.size() * sizeof(GLuint),
                              &indices[0], GL_STATIC_DRAW));

        mIndexBufferSize = indices.size();
    }

    Mesh::~Mesh()
    {
        if (mIndexBuffer) {
            GL_CHECK(glDeleteBuffers(1, &mIndexBuffer));
        }
    }
} // namespace sb

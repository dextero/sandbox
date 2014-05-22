#include "mesh.h"

#include "utils/libUtils.h"
#include "utils/logger.h"


namespace sb
{
    SharedVertexBuffer* Mesh::msBuffer = NULL;


    Mesh::Mesh():
        mBufferOffset((uint32_t)-1),
        mBufferSize(0u),
        mIndexBuffer(0),
        mIndexBufferSize(0u)
    {
        assert(msBuffer && "Mesh::msBuffer should be already initialized. Use ResourceMgr to create Mesh objects.");
    }

    Mesh::~Mesh()
    {
        assert(msBuffer);

        if (mBufferSize)
            msBuffer->releaseVertices(mBufferOffset);
        if (mIndexBuffer)
            GL_CHECK(glDeleteBuffers(1, &mIndexBuffer));
    }

    bool Mesh::create(EShape shape,
                      const std::vector<Vec3>& vertices,
                      const std::vector<Vec2>& texcoords,
                      const std::vector<Color>& colors,
                      const std::vector<uint32_t>& indices,
                      std::shared_ptr<TextureId> texture)
    {
        assert(msBuffer);

        mShape = shape;
        mTexture = texture;

        // vertices
        mBufferOffset = msBuffer->addVertices(&vertices[0],
                                              texcoords.size() ? &texcoords[0] : NULL,
                                              colors.size() ? &colors[0] : NULL,
                                              vertices.size());
        mBufferSize = vertices.size();

        // indices
        if (mBufferSize == 0) {
            gLog.warn("invalid mesh vertex data\n");
            return false;
        } else {
            std::vector<uint32_t> adjustedIndices = indices;
            for (size_t i = 0; i < indices.size(); ++i) {
                adjustedIndices[i] += mBufferOffset;
            }

            // index buffer
            GL_CHECK(glGenBuffers(1, &mIndexBuffer));
            GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer));
            GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                                  indices.size() * sizeof(GLuint),
                                  &adjustedIndices[0], GL_STATIC_DRAW));

            mIndexBufferSize = indices.size();
        }

        return true;
    }
} // namespace sb

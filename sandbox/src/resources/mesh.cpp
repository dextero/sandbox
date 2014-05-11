#include "mesh.h"

#include "utils/libUtils.h"
#include "utils/profiler.h"
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
            msBuffer->ReleaseVertices(mBufferOffset);
        if (mIndexBuffer)
            GL_CHECK(glDeleteBuffers(1, &mIndexBuffer));
    }

    bool Mesh::Create(EShape shape, Vec3* vertices, Vec2* texcoords, Color* colors, uint32_t elements, uint32_t* indices, uint32_t numIndices, uint32_t textureId)
    {
        assert(msBuffer);

        mShape = shape;
        mTexture = textureId;

        // vertices
        mBufferOffset = msBuffer->AddVertices(vertices, texcoords, colors, elements);
        mBufferSize = elements;

        // indices
        if (!mBufferSize)
        {
            gLog.Warn("invalid mesh vertex data\n");
            return false;
        }
        else
        {
            for (uint32_t i = 0; i < numIndices; ++i)
                indices[i] += mBufferOffset;

            // index buffer
            GL_CHECK(glGenBuffers(1, &mIndexBuffer));
            GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer));
            GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), indices, GL_STATIC_DRAW));

            mIndexBufferSize = numIndices;
        }

        return true;
    }

    SharedVertexBuffer& Mesh::GetVertexBuffer()
    {
        assert(msBuffer);
        return *msBuffer;
    }

    uint32_t Mesh::GetVertexBufferOffset()
    {
        return mBufferOffset;
    }

    BufferId Mesh::GetIndexBuffer()
    {
        return mIndexBuffer;
    }

    uint32_t Mesh::GetIndexBufferSize()
    {
        return mIndexBufferSize;
    }

    Mesh::EShape Mesh::GetShape()
    {
        return mShape;
    }

    TextureId Mesh::GetTexture()
    {
        return mTexture;
    }
} // namespace sb

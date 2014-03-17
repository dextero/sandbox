#ifndef MESH_H
#define MESH_H

#include "rendering/types.h"
#include "rendering/color.h"
#include "utils/types.h"
#include "rendering/sharedVertexBuffer.h"

namespace sb
{
    class Mesh
    {
    public:
        enum EShape {
            ShapePoint = SHAPE_POINTS,
            ShapeLine = SHAPE_LINES,
            ShapeTriangle = SHAPE_TRIANGLES,
            ShapeQuad = SHAPE_QUADS,
            ShapeTriangleStrip = SHAPE_TRIANGLE_STRIP
        };

    private:
        static SharedVertexBuffer* msBuffer;
        uint32_t mBufferOffset, mBufferSize;

        BufferId mIndexBuffer;
        uint32_t mIndexBufferSize;

        EShape mShape;
        TextureId mTexture;

        Mesh();
        ~Mesh();

        bool Create(EShape shape, Vec3* vertices, Vec2* texcoords, Color* colors, uint32_t elements, uint32_t* indices, uint32_t numIndices, uint32_t textureId);

    public:
        static SharedVertexBuffer& GetVertexBuffer();
        uint32_t GetVertexBufferOffset();
        BufferId GetIndexBuffer();
        uint32_t GetIndexBufferSize();
        EShape GetShape();
        TextureId GetTexture();

        friend class ResourceMgr;
    };
} // namespace sb

#endif // MESH_H

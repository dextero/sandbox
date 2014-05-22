#ifndef MESH_H
#define MESH_H

#include <memory>
#include <vector>

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
        uint32_t mBufferOffset;
        uint32_t mBufferSize;

        BufferId mIndexBuffer;
        uint32_t mIndexBufferSize;

        EShape mShape;
        std::shared_ptr<TextureId> mTexture;

        bool Create(EShape shape,
                    const std::vector<Vec3>& vertices,
                    const std::vector<Vec2>& texcoords,
                    const std::vector<Color>& colors,
                    const std::vector<uint32_t>& indices,
                    std::shared_ptr<TextureId> texture);

    public:
        Mesh();
        ~Mesh();

        static SharedVertexBuffer& GetVertexBuffer()
        {
            assert(msBuffer);
            return *msBuffer;
        }

        uint32_t GetVertexBufferOffset() { return mBufferOffset; }
        BufferId GetIndexBuffer() { return mIndexBuffer; }
        uint32_t GetIndexBufferSize() { return mIndexBufferSize; }

        EShape GetShape() { return mShape; }
        const std::shared_ptr<TextureId>& GetTexture() { return mTexture; }

        void SetTexture(const std::shared_ptr<TextureId>& texture)
        {
            mTexture = texture;
        }

        friend class ResourceMgr;
    };
} // namespace sb

#endif // MESH_H

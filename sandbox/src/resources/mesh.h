#ifndef MESH_H
#define MESH_H

#include <memory>
#include <vector>

#include "rendering/types.h"
#include "rendering/color.h"
#include "utils/types.h"
#include "rendering/vertexBuffer.h"

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

        Mesh(EShape shape,
             const std::vector<Vec3>& vertices,
             const std::vector<Vec2>& texcoords,
             const std::vector<Color>& colors,
             const std::vector<uint32_t>& indices,
             std::shared_ptr<TextureId> texture);

        VertexBuffer& getVertexBuffer() { return mVertexBuffer; }
        IndexBuffer& getIndexBuffer() { return mIndexBuffer; }
        size_t getIndexBufferSize() { return mIndexBufferSize; }

        EShape getShape() { return mShape; }
        const std::shared_ptr<TextureId>& getTexture() { return mTexture; }

        void setTexture(const std::shared_ptr<TextureId>& texture)
        {
            mTexture = texture;
        }

    private:
        VertexBuffer mVertexBuffer;
        IndexBuffer mIndexBuffer;
        uint32_t mIndexBufferSize;

        EShape mShape;
        std::shared_ptr<TextureId> mTexture;

        friend class ResourceMgr;
    };
} // namespace sb

#endif // MESH_H

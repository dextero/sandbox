#ifndef MESH_H
#define MESH_H

#include <memory>
#include <vector>

#include <sandbox/rendering/types.h>
#include <sandbox/rendering/color.h>
#include <sandbox/rendering/texture.h>
#include <sandbox/rendering/vertexBuffer.h>
#include <sandbox/utils/types.h>

namespace sb
{
    class Mesh
    {
    public:
        enum Shape {
            Point = SHAPE_POINTS,
            Line = SHAPE_LINES,
            LineStrip = SHAPE_LINE_STRIP,
            Triangle = SHAPE_TRIANGLES,
            TriangleStrip = SHAPE_TRIANGLE_STRIP
        };

        Mesh(Shape shape,
             const std::vector<Vec3>& vertices,
             const std::vector<Vec2>& texcoords,
             const std::vector<Color>& colors,
             const std::vector<Vec3>& normals,
             const std::vector<uint32_t>& indices,
             std::shared_ptr<Texture> texture);

        VertexBuffer& getVertexBuffer() { return mVertexBuffer; }
        IndexBuffer& getIndexBuffer() { return mIndexBuffer; }
        size_t getIndexBufferSize() { return mIndexBufferSize; }

        Shape getShape() { return mShape; }
        const std::shared_ptr<Texture>& getTexture() { return mTexture; }

        void setTexture(const std::shared_ptr<Texture>& texture)
        {
            mTexture = texture;
        }

    private:
        VertexBuffer mVertexBuffer;
        IndexBuffer mIndexBuffer;
        uint32_t mIndexBufferSize;

        Shape mShape;
        std::shared_ptr<Texture> mTexture;

        friend class ResourceMgr;
    };
} // namespace sb

#endif // MESH_H

#ifndef SHAREDVERTEXBUFFER_H
#define SHAREDVERTEXBUFFER_H

#include "types.h"
#include "utils/types.h"
#include "utils/lib.h"
#include "utils/misc.h"
#include "utils/logger.h"
#include "rendering/color.h"
#include "rendering/buffer.h"
#include "rendering/indexBuffer.h"

#include <vector>

namespace sb
{
    class VertexBuffer
    {
    public:
        class Type {
        public:
            static const Type& Vertex;
            static const Type& Color;
            static const Type& Texcoord;
            static const Type& Normal;

            static const size_t COUNT = 4;

            const uint32_t attribIdx;
            const size_t elemSize;
        private:
            Type(uint32_t attribIdx,
                 size_t elemSize):
                attribIdx(attribIdx),
                elemSize(elemSize)
            {}
        };

        VertexBuffer(const std::vector<Vec3>& vertices,
                     const std::vector<Vec2>& texcoords,
                     const std::vector<Color>& colors,
                     const std::vector<Vec3>& normals);
        VertexBuffer(const VertexBuffer& copy) = delete;
        ~VertexBuffer();

        void bind() const;
        void unbind() const;

        void debug();

    private:
        struct BufferTypePair {
            Buffer buffer;
            Type type;

            BufferTypePair(Buffer&& buffer,
                           Type type):
                buffer(std::forward<Buffer>(buffer)),
                type(type)
            {}
        };

        static const uint32_t SizeofElem[];

        BufferId mVAO;
        std::vector<BufferTypePair> mBuffers;

        void addBuffer(const Type& type,
                       const void* data,
                       size_t numElements);
    };
} // namespace sb

#endif //SHAREDVERTEXBUFFER_H

#ifndef SHAREDVERTEXBUFFER_H
#define SHAREDVERTEXBUFFER_H

#include "types.h"
#include "../utils/types.h"
#include "../rendering/color.h"

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

            static const size_t COUNT = 3;

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
                     const std::vector<Color>& colors);
        VertexBuffer(const VertexBuffer& copy) = delete;
        ~VertexBuffer();

        void bind() const;
        void unbind() const;
#ifdef TODO_IS_THIS_EVEN_NECESSARY
        bool empty() const;
        bool hasBuffer(EBufferType type) const;

        void debug();
#endif

    private:
        struct Buffer {
            BufferId id;
            Type type;
        };

        static const uint32_t SizeofElem[];

        BufferId mVAO;
        std::vector<Buffer> mBuffers;

        void addBuffer(const Type& type,
                       const void* data,
                       size_t numElements);
    };
} // namespace sb

#endif //SHAREDVERTEXBUFFER_H

#ifndef SHAREDVERTEXBUFFER_H
#define SHAREDVERTEXBUFFER_H

#include "types.h"
#include "../utils/types.h"

#include <list>
#include <map>

namespace sb
{
    class SharedVertexBuffer
    {
    public:
        enum EBufferType {
            BufferVertex = 0,
            BufferColor,
            BufferTexcoord,
            BufferCount
        };

    private:
        static const uint32_t SizeofElem[];

        BufferId mVAO;
        BufferId mBuffers[BufferCount];

        // 0 for BufferVertex, 1 for BufferVertex and BufferColor etc.
        EBufferType mActiveBuffers;

        struct SChunk {
            uint32_t offset;    // in elements
            uint32_t size;        // in elements

            SChunk(uint32_t offset, uint32_t size): offset(offset), size(size) {}
        };
        std::list<SChunk> mEmptyChunks;
        std::map<uint32_t, uint32_t> mUsedChunks;

        void CopyBufferData(BufferId from, BufferId to, uint32_t bytes);
        void ExpandBuffers(uint32_t elemsNeeded);

    public:
        SharedVertexBuffer(EBufferType activeBuffers = BufferVertex, uint32_t initialSizeElems = 1024);
        SharedVertexBuffer(const SharedVertexBuffer& copy);
        ~SharedVertexBuffer();

        void AddElements(EBufferType buffer, const void* data, uint32_t elements, uint32_t offset);
        uint32_t AddVertices(const void* vertices, const void* texcoords, const void* colors, uint32_t elements);
        void ReleaseVertices(uint32_t offset);

        void Bind() const;
        void Unbind() const;
        bool Empty() const;
        bool HasBuffer(EBufferType type) const;

        void Debug();
    };
} // namespace sb

#endif //SHAREDVERTEXBUFFER_H

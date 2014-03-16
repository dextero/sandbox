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
        static const uint SizeofElem[];

        BufferId mVAO;
        BufferId mBuffers[BufferCount];

        // 0 for BufferVertex, 1 for BufferVertex and BufferColor etc.
        EBufferType mActiveBuffers;

        struct SChunk {
            uint offset;    // in elements
            uint size;        // in elements

            SChunk(uint offset, uint size): offset(offset), size(size) {}
        };
        std::list<SChunk> mEmptyChunks;
        std::map<uint, uint> mUsedChunks;

        void CopyBufferData(BufferId from, BufferId to, uint bytes);
        void ExpandBuffers(uint elemsNeeded);

    public:
        SharedVertexBuffer(EBufferType activeBuffers = BufferVertex, uint initialSizeElems = 1024);
        SharedVertexBuffer(const SharedVertexBuffer& copy);
        ~SharedVertexBuffer();

        void AddElements(EBufferType buffer, void* data, uint elements, uint offset);
        uint AddVertices(void* vertices, void* texcoords, void* colors, uint elements);
        void ReleaseVertices(uint offset);

        void Bind() const;
        void Unbind() const;
        bool Empty() const;
        bool HasBuffer(EBufferType type) const;

        void Debug();
    };
} // namespace sb

#endif //SHAREDVERTEXBUFFER_H

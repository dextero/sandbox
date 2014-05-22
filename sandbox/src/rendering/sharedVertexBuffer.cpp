#include "sharedVertexBuffer.h"
#include "color.h"

#include "../utils/libUtils.h"
#include "../utils/logger.h"

#include <cassert>
#include <algorithm>

// damn you windows.h
#ifdef min
#   undef min
#endif

namespace sb
{
    const uint32_t SharedVertexBuffer::SizeofElem[] = {
        3,    // Vertex
        4,    // Color
        2    // Texcoord
    };


    void SharedVertexBuffer::copyBufferData(BufferId from, BufferId to, uint32_t bytes)
    {
        if (glCopyBufferSubData)
        {
            GL_CHECK(glBindBuffer(GL_COPY_READ_BUFFER, from));
            GL_CHECK(glBindBuffer(GL_COPY_WRITE_BUFFER, to));
            GL_CHECK(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, bytes));
            GL_CHECK(glBindBuffer(GL_COPY_READ_BUFFER, 0));
            GL_CHECK(glBindBuffer(GL_COPY_WRITE_BUFFER, 0));
        }
        else
        {
            char* buffer = new char[bytes];

            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, from));
            GL_CHECK(glGetBufferSubData(GL_ARRAY_BUFFER, 0, bytes, buffer));
            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, to));
            GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, buffer));
            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

            delete[] buffer;
        }
    }

    void SharedVertexBuffer::expandBuffers(uint32_t elemsNeeded)
    {
        gLog.info("expanding buffer, needed: %u elements\n", elemsNeeded);

        GLuint newBuffers[BufferCount];
        GLint vbufSize = 0;
        GLint vbufOldSize = 0;
        GL_CHECK(glGenBuffers(BufferCount, newBuffers));

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mBuffers[BufferVertex]));
        GL_CHECK(glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &vbufOldSize));

        vbufOldSize /= SizeofElem[BufferVertex];
        vbufSize = vbufOldSize;
        while ((uint32_t)(vbufSize - vbufOldSize) < elemsNeeded)
            vbufSize *= 2;

        gLog.info("current buffer size: %u, new size: %u\n", vbufOldSize, vbufSize);

        GL_CHECK(glBindVertexArray(mVAO));
        for (uint32_t i = 0; i < BufferCount; ++i)
        {
            if (!hasBuffer((EBufferType)i))
            {
                newBuffers[i] = 0;
                continue;
            }

            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, newBuffers[i]));
            GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vbufSize * SizeofElem[i] * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW));
            GL_CHECK(glVertexAttribPointer(i, SizeofElem[i], GL_FLOAT, GL_FALSE, 0, NULL));

            copyBufferData(mBuffers[i], newBuffers[i], vbufOldSize * SizeofElem[i]);
        }
        GL_CHECK(glBindVertexArray(0));

        GL_CHECK(glDeleteBuffers(BufferCount, mBuffers));
        for (uint32_t i = 0; i < BufferCount; ++i)
            mBuffers[i] = newBuffers[i];

        std::list<SChunk>::iterator it = mEmptyChunks.begin();
        for (; it != mEmptyChunks.end(); ++it)
            if (it->offset + it->size == (uint32_t)vbufOldSize)
            {
                it->size += vbufSize - vbufOldSize;
                break;
            }

        if (it == mEmptyChunks.end())
            mEmptyChunks.push_back(SChunk(vbufOldSize, vbufSize - vbufOldSize));
    }

    SharedVertexBuffer::SharedVertexBuffer(EBufferType activeBuffers, uint32_t initialSizeElems):
        mVAO(0),
        mActiveBuffers(activeBuffers)
    {
        gLog.info("creating shared vertex buffer\n");

        GL_CHECK(glGenVertexArrays(1, &mVAO));
        GL_CHECK(glBindVertexArray(mVAO));

        // GL doesn't create buffer of desired size, but _at least_ that size
        uint32_t realSizeElems = 0;

        GL_CHECK(glGenBuffers(BufferCount, mBuffers));
        for (uint32_t i = BufferVertex; i < BufferCount; ++i)
        {
            if (!hasBuffer((EBufferType)i))
            {
                mBuffers[i] = 0;
                continue;
            }

            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mBuffers[i]));
            GL_CHECK(glBufferData(GL_ARRAY_BUFFER, initialSizeElems * SizeofElem[i] * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW));
            GL_CHECK(glVertexAttribPointer(i, SizeofElem[i], GL_FLOAT, GL_FALSE, 0, NULL));

            if (!realSizeElems)
                GL_CHECK(glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, (GLint*)&realSizeElems));
        }

        mEmptyChunks.push_back(SChunk(0, realSizeElems));
    }

    SharedVertexBuffer::SharedVertexBuffer(const SharedVertexBuffer& copy)
    {
        *this = copy;
        // copying should be made ONLY while *moving* (STL containers), so...
        ((SharedVertexBuffer&)copy).mVAO = 0;
    }

    SharedVertexBuffer::~SharedVertexBuffer()
    {
        if (mVAO)
        {
            GLint current;
            GL_CHECK(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current));
            if ((GLuint)current == mVAO)
                GL_CHECK(glBindVertexArray(0));

            GL_CHECK(glDeleteVertexArrays(1, &mVAO));
        }
    }

    void SharedVertexBuffer::addElements(EBufferType buffer, const void* data, uint32_t elements, uint32_t offset)
    {
        GLfloat* placeholderBuffer = NULL;
        if (!data)
        {
            placeholderBuffer = new GLfloat[elements * SizeofElem[buffer]];
            for (uint32_t i = 0; i < elements * SizeofElem[buffer]; ++i)
                placeholderBuffer[i] = 1.f;
        }

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mBuffers[buffer]));
        GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, offset * SizeofElem[buffer] * sizeof(GLfloat), elements * SizeofElem[buffer] * sizeof(GLfloat), data ? data : placeholderBuffer));
        delete[] placeholderBuffer;
    }

    uint32_t SharedVertexBuffer::addVertices(const void* vertices, const void* texcoords, const void* colors, uint32_t elements)
    {
        gLog.info("adding %u elements to shared buffer\n", elements);

        if (!elements) return (uint32_t)-1;

        std::list<SChunk>::iterator it = mEmptyChunks.begin();
        while (it != mEmptyChunks.end() && it->size < elements) ++it;

        if (it == mEmptyChunks.end())
        {
            expandBuffers(elements);
            it = mEmptyChunks.begin();
        }

        uint32_t offset = it->offset;
        gLog.info("using offset %u\n", offset);

        // vertices
        if (hasBuffer(BufferVertex))
            addElements(BufferVertex, vertices, elements, offset);
        // colors
        if (hasBuffer(BufferColor))
            addElements(BufferColor, colors, elements, offset);
        // texcoords
        if (hasBuffer(BufferTexcoord))
            addElements(BufferTexcoord, texcoords, elements, offset);

        if (it->size == elements)
            mEmptyChunks.erase(it);
        else
        {
            it->offset += elements;
            it->size -= elements;
        }

        mUsedChunks.insert(std::make_pair(offset, elements));
        //debug();

        return offset;
    }

    void SharedVertexBuffer::releaseVertices(uint32_t offset)
    {
        uint32_t sizeElements = mUsedChunks[offset];
        gLog.info("releasing %u elements at offset %u\n", sizeElements, offset);

        mUsedChunks.erase(offset);

        std::list<SChunk>::iterator it, joined = mEmptyChunks.end();
        for (it = mEmptyChunks.begin(); it != mEmptyChunks.end(); ++it)
        {
            if ((it->offset + it->size == offset) || (sizeElements + offset == it->offset))
            {
                // released chunk is just after another empty - join them
                if (joined != mEmptyChunks.end() && ((joined->offset + joined->size == it->offset) || (it->offset + it->size == joined->offset)))
                {
                    it->size += joined->size;
                    it->offset = std::min(it->offset, joined->offset);
                    mEmptyChunks.erase(joined);
                    // both ends joined, we're done
                    return;
                }

                it->size += sizeElements;
                it->offset = std::min(offset, it->offset);
            }
        }

        // nothing found?
        mEmptyChunks.push_back(SChunk(offset, sizeElements));
    }

    void SharedVertexBuffer::bind() const
    {
        GL_CHECK(glBindVertexArray(mVAO));
    }

    void SharedVertexBuffer::unbind() const
    {
        GL_CHECK(glBindVertexArray(0));
    }

    bool SharedVertexBuffer::empty() const
    {
        return mUsedChunks.empty();
    }

    bool SharedVertexBuffer::hasBuffer(EBufferType type) const
    {
        return mActiveBuffers >= type;
    }

    void SharedVertexBuffer::debug()
    {
        GL_CHECK(glBindVertexArray(mVAO));

        for (uint32_t i = 0; i < BufferCount; ++i)
        {
            if (!hasBuffer((EBufferType)i))
                continue;

            GLint size;
            GLuint attribBuffer = 0;

            GL_CHECK(glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, (GLint*)&attribBuffer));
            assert((attribBuffer == mBuffers[i]) && "Invalid attrib array buffer binding!");

            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mBuffers[i]));
            GL_CHECK(glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size));
            assert((attribBuffer == mBuffers[i]) && "Invalid attrib array buffer size!");
        }

        GL_CHECK(glBindVertexArray(0));
    }
} // namespace sb

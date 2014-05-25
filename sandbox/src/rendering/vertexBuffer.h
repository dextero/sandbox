#ifndef SHAREDVERTEXBUFFER_H
#define SHAREDVERTEXBUFFER_H

#include "types.h"
#include "utils/types.h"
#include "utils/lib.h"
#include "utils/misc.h"
#include "utils/logger.h"
#include "rendering/color.h"

#include <vector>

namespace sb
{
    class Buffer
    {
    public:
        Buffer(const void* data,
               size_t bytes):
            id(0),
            bufferType(0),
            prevId(0)
        {
            assert(bytes > 0 && "added buffer must not be empty");

            gLog.info("creating buffer: %lu bytes\n", bytes);

            GL_CHECK(glGenBuffers(1, &id));

            {
                auto bind = make_bind(*this, GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING);

                GL_CHECK(glBufferData(GL_ARRAY_BUFFER, bytes, data,
                                      GL_DYNAMIC_DRAW));
            }
        }

        Buffer(const Buffer&) = delete;
        Buffer& operator =(const Buffer&) = delete;

        Buffer(Buffer&& old):
            id(old.id),
            bufferType(old.bufferType),
            prevId(old.prevId)
        {
            old.id = 0;
            old.bufferType = 0;
            old.prevId = 0;
        }

        Buffer& operator =(Buffer&& old)
        {
            if (id) {
                unbind();
                GL_CHECK(glDeleteBuffers(1, &id));
            }

            id = old.id;
            bufferType = old.bufferType;
            prevId = old.prevId;

            old.id = 0;
            old.bufferType = 0;
            old.prevId = 0;

            return *this;
        }

        ~Buffer()
        {
            if (id) {
                GL_CHECK(glDeleteBuffers(1, &id));
            }
        }

        void bind(GLuint bufferType,
                  GLuint bufferBinding)
        {
            assert(this->prevId == 0
                   && this->bufferType == 0
                   && "recursive bind? this should never happen");
            this->bufferType = bufferType;
            GL_CHECK(glGetIntegerv(bufferBinding, (GLint*)&prevId));
            GL_CHECK(glBindBuffer(bufferType, id));

            gLog.debug("buffer %x: bind %d (was %d)\n", bufferType, prevId, id);
        }

        void unbind()
        {
            gLog.debug("unbind %x\n", bufferType);

            GL_CHECK(glBindBuffer(bufferType, prevId));
            bufferType = 0;
            prevId = 0;
        }

        BufferId getId() const { return id; }

    private:
        BufferId id;

        GLuint bufferType;
        BufferId prevId;
    };

    class IndexBuffer: public Buffer
    {
    public:
        IndexBuffer(const std::vector<uint32_t>& indices):
            Buffer(&indices[0], indices.size() * sizeof(uint32_t))
        {}

        void bind()
        {
            Buffer::bind(GL_ELEMENT_ARRAY_BUFFER,
                         GL_ELEMENT_ARRAY_BUFFER_BINDING);
        }
    };

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

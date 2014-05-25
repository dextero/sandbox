#ifndef RENDERING_BUFFER_H
#define RENDERING_BUFFER_H

#include "rendering/types.h"

namespace sb
{
    class Buffer
    {
    public:
        Buffer(const void* data,
               size_t bytes);

        Buffer(const Buffer&) = delete;
        Buffer& operator =(const Buffer&) = delete;

        Buffer(Buffer&& old);
        Buffer& operator =(Buffer&& old);

        ~Buffer();

        void bind(GLuint bufferType,
                  GLuint bufferBinding);
        void unbind();

        BufferId getId() const { return id; }

    private:
        BufferId id;

        GLuint bufferType;
        BufferId prevId;
    };
} // namespace sb

#endif /* RENDERING_BUFFER_H */

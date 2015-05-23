#ifndef RENDERING_BUFFER_H
#define RENDERING_BUFFER_H

#include <sandbox/rendering/types.h>

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

        virtual ~Buffer();

        void bind(GLuint bufferType,
                  GLuint bufferBinding) const;
        void unbind() const;

        BufferId getId() const { return id; }

    private:
        BufferId id;

        mutable GLuint bufferType;
        mutable BufferId prevId;
    };
} // namespace sb

#endif /* RENDERING_BUFFER_H */

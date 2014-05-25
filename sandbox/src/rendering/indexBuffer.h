#ifndef RENDERING_INDEXBUFFER_H
#define RENDERING_INDEXBUFFER_H

#include "rendering/buffer.h"

#include <vector>

namespace sb
{
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
} // namespace sb

#endif /* RENDERING_INDEXBUFFER_H */

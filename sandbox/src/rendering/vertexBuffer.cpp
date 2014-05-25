#include "rendering/vertexBuffer.h"
#include "rendering/color.h"

#include "utils/lib.h"
#include "utils/logger.h"

#include <cassert>
#include <algorithm>

// damn you windows.h
#ifdef min
#   undef min
#endif

namespace sb
{
    const VertexBuffer::Type& VertexBuffer::Type::Vertex = { 0, sizeof(Vec3) };
    const VertexBuffer::Type& VertexBuffer::Type::Color = { 1, sizeof(Color) };
    const VertexBuffer::Type& VertexBuffer::Type::Texcoord = { 2, sizeof(Vec2) };

    void VertexBuffer::addBuffer(const Type& type,
                                 const void* data,
                                 size_t numElements)
    {
        Buffer buffer(data, numElements * type.elemSize);

        buffer.bind(GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING);
        GL_CHECK(glVertexAttribPointer(type.attribIdx,
                                       type.elemSize / sizeof(float), GL_FLOAT,
                                       GL_FALSE, 0, NULL));
        buffer.unbind();

        mBuffers.emplace_back(std::move(buffer), type);
    }

    VertexBuffer::VertexBuffer(const std::vector<Vec3>& vertices,
                               const std::vector<Vec2>& texcoords,
                               const std::vector<Color>& colors):
        mVAO(0)
    {
        gLog.info("creating vertex buffer (%lu, vertices%s%s)\n",
                  vertices.size(),
                  texcoords.size() > 0 ? " texcoords" : "",
                  colors.size() > 0 ? " colors" : "");

        GL_CHECK(glGenVertexArrays(1, &mVAO));
        GL_CHECK(glBindVertexArray(mVAO));

        assert(vertices.size() > 0 && "vertex buffer must have some vertices");

        addBuffer(Type::Vertex, &vertices[0], vertices.size());

        if (texcoords.size() > 0) {
            if (vertices.size() != texcoords.size()) {
                gLog.warn("%lu vertices, but %lu texcoords\n",
                          vertices.size(), texcoords.size());
            }
            addBuffer(Type::Texcoord, &texcoords[0], texcoords.size());
        }

        if (colors.size() > 0) {
            if (vertices.size() != colors.size()) {
                gLog.warn("%lu vertices, but %lu colors\n",
                          vertices.size(), colors.size());
            }
            addBuffer(Type::Color, &colors[0], colors.size());
        }
    }

    VertexBuffer::~VertexBuffer()
    {
        if (mVAO)
        {
            GLint current;
            GL_CHECK(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current));
            if ((GLuint)current == mVAO) {
                GL_CHECK(glBindVertexArray(0));
            }

            for (const BufferTypePair& bt: mBuffers) {
                // TODO: unbind?
                GLuint id = bt.buffer.getId();
                GL_CHECK(glDeleteBuffers(1, &id));
            }

            GL_CHECK(glDeleteVertexArrays(1, &mVAO));
        }
    }

    void VertexBuffer::bind() const
    {
        GL_CHECK(glBindVertexArray(mVAO));
    }

    void VertexBuffer::unbind() const
    {
        GL_CHECK(glBindVertexArray(0));
    }

#ifdef TODO_IS_THIS_EVEN_NECESSARY
    bool VertexBuffer::empty() const
    {
        return mUsedChunks.empty();
    }

    bool VertexBuffer::hasBuffer(const Type& type) const
    {
        return std::find_if(mBuffers, [](const Buffer& b) { b.type == type; });
    }

    void VertexBuffer::debug()
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
#endif
} // namespace sb

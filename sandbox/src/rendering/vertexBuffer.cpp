#include <sandbox/rendering/vertexBuffer.h>
#include <sandbox/rendering/color.h>

#include <sandbox/utils/lib.h>
#include <sandbox/utils/logger.h>
#include <sandbox/utils/debug.h>

#include <algorithm>

// damn you windows.h
#ifdef min
#   undef min
#endif

namespace sb {

const std::map<Attrib::Kind, Attrib> ATTRIBS {
    { Attrib::Kind::Position, { "position", GL_FLOAT, 3, sizeof(Vec3) } },
    { Attrib::Kind::Texcoord, { "texcoord", GL_FLOAT, 2, sizeof(Vec2) } },
    { Attrib::Kind::Color,    { "color",    GL_FLOAT, 4, sizeof(Vec4) } },
    { Attrib::Kind::Normal,   { "normal",   GL_FLOAT, 3, sizeof(Vec3) } }
};

void VertexBuffer::addBuffer(const Attrib::Kind& kind,
                             const void* data,
                             size_t numElements)
{
    const Attrib& attrib = ATTRIBS.find(kind)->second;
    Buffer buffer(data, numElements * attrib.elemSizeBytes);

    buffer.bind(GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING);
    GL_CHECK(glEnableVertexAttribArray(mBuffers.size()));
    GL_CHECK(glVertexAttribPointer(mBuffers.size(),
                                   attrib.numComponents, GL_FLOAT,
                                   GL_FALSE, 0, NULL));
    buffer.unbind();

    mBuffers.emplace_back(std::move(buffer), kind);
}

VertexBuffer::VertexBuffer(const std::vector<Vec3>& vertices,
                           const std::vector<Vec2>& texcoords,
                           const std::vector<Color>& colors,
                           const std::vector<Vec3>& normals):
    mVAO(0),
    mBuffers()
{
#if 0
    gLog.debug("creating vertex buffer (%lu, vertices%s%s)\n",
               vertices.size(),
               texcoords.size() > 0 ? " texcoords" : "",
               colors.size() > 0 ? " colors" : "");
#endif

    GL_CHECK(glGenVertexArrays(1, &mVAO));
    auto vaoBind = make_bind(*this);

    sbAssert(vertices.size() > 0, "vertex buffer must have some vertices");
    addBuffer(Attrib::Kind::Position, &vertices[0], vertices.size());

    if (texcoords.size() > 0) {
        if (vertices.size() != texcoords.size()) {
            gLog.warn("%lu vertices, but %lu texcoords\n",
                      vertices.size(), texcoords.size());
        }
        addBuffer(Attrib::Kind::Texcoord, &texcoords[0], texcoords.size());
    }

    if (colors.size() > 0) {
        if (vertices.size() != colors.size()) {
            gLog.warn("%lu vertices, but %lu colors\n",
                      vertices.size(), colors.size());
        }
        addBuffer(Attrib::Kind::Color, &colors[0], colors.size());
    }

    if (normals.size() > 0) {
        if (vertices.size() != normals.size()) {
            gLog.warn("%lu vertices, but %lu normals\n",
                      vertices.size(), normals.size());
        }
        addBuffer(Attrib::Kind::Normal, &normals[0], normals.size());
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

        for (const BufferKindPair& bk: mBuffers) {
            // TODO: unbind?
            GLuint id = bk.buffer.getId();
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

void VertexBuffer::debug()
{
    gLog.debug("VAO %d: %lu buffers\n", mVAO, mBuffers.size());
    for (size_t i = 0; i < mBuffers.size(); ++i) {
        GLuint attribBuffer = 0;

        GL_CHECK(glGetVertexAttribiv((GLuint)i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, (GLint*)&attribBuffer));

        GLint size;
        auto bufferBind = make_bind(mBuffers[i].buffer, GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING);
        GL_CHECK(glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size));
        gLog.debug("- attrib %lu: buffer %d (%d bytes)\n", i, attribBuffer, size);
    }
}

} // namespace sb

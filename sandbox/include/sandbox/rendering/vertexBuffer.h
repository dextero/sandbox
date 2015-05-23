#ifndef SHAREDVERTEXBUFFER_H
#define SHAREDVERTEXBUFFER_H

#include <sandbox/rendering/types.h>
#include <sandbox/utils/types.h>
#include <sandbox/utils/lib.h>
#include <sandbox/utils/misc.h>
#include <sandbox/utils/logger.h>
#include <sandbox/rendering/color.h>
#include <sandbox/rendering/buffer.h>
#include <sandbox/rendering/indexBuffer.h>

#include <vector>
#include <map>

namespace sb
{
    struct Attrib {
        enum class Kind {
            Unspecified,
            Position,
            Texcoord,
            Color,
            Normal,
        };

        std::string kindAsString;
        GLuint componentType;
        size_t numComponents;
        size_t elemSizeBytes;
    };

    extern const std::map<Attrib::Kind, Attrib> ATTRIBS;

    struct BufferKindPair {
        Buffer buffer;
        Attrib::Kind kind;

        BufferKindPair(Buffer&& buffer,
                       Attrib::Kind kind):
            buffer(std::forward<Buffer>(buffer)),
            kind(kind)
        {}
    };

    class VertexBuffer
    {
    public:
        VertexBuffer(const std::vector<Vec3>& vertices,
                     const std::vector<Vec2>& texcoords,
                     const std::vector<Color>& colors,
                     const std::vector<Vec3>& normals);
        VertexBuffer(const VertexBuffer& copy) = delete;
        ~VertexBuffer();

        const std::vector<BufferKindPair>& getBuffers() const
        {
            return mBuffers;
        }

        void bind() const;
        void unbind() const;

        void debug();

    private:
        BufferId mVAO;
        std::vector<BufferKindPair> mBuffers;

    void addBuffer(const Attrib::Kind& kind,
                   const void* data,
                   size_t numElements);
    };
} // namespace sb

#endif //SHAREDVERTEXBUFFER_H

#include "rendering/buffer.h"

#include "utils/lib.h"
#include "utils/logger.h"
#include "utils/misc.h"
#include "utils/stringUtils.h"

#include <cassert>
#include <map>

namespace sb
{
    Buffer::Buffer(const void* data,
                   size_t bytes):
        id(0),
        bufferType(0),
        prevId(0)
    {
        assert(bytes > 0 && "added buffer must not be empty");

        gLog.trace("creating buffer: %lu bytes\n", bytes);

        GL_CHECK(glGenBuffers(1, &id));

        {
            auto bind = make_bind(*this, GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING);

            GL_CHECK(glBufferData(GL_ARRAY_BUFFER, bytes, data,
                                  GL_DYNAMIC_DRAW));
        }
    }

    Buffer::Buffer(Buffer&& old):
        id(old.id),
        bufferType(old.bufferType),
        prevId(old.prevId)
    {
        old.id = 0;
        old.bufferType = 0;
        old.prevId = 0;
    }

    Buffer& Buffer::operator =(Buffer&& old)
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

    Buffer::~Buffer()
    {
        if (id) {
            GL_CHECK(glDeleteBuffers(1, &id));
        }
    }

    namespace
    {
        std::string getBufferName(GLuint bufferType)
        {
            static const std::map<GLuint, std::string> BUFFER_TYPES {
#define BUFFER(type) { type, #type }
                BUFFER(GL_ARRAY_BUFFER),
                BUFFER(GL_ELEMENT_ARRAY_BUFFER)
#undef BUFFER
            };

            auto it = BUFFER_TYPES.find(bufferType);
            if (it == BUFFER_TYPES.end()) {
                return utils::makeString(std::hex, bufferType);
            }
            return it->second;
        }
    } // namespace

    void Buffer::bind(GLuint bufferType,
                      GLuint bufferBinding)
    {
        assert(this->prevId == 0
               && this->bufferType == 0
               && "recursive bind? this should never happen");
        this->bufferType = bufferType;
        GL_CHECK(glGetIntegerv(bufferBinding, (GLint*)&prevId));
        GL_CHECK(glBindBuffer(bufferType, id));

        gLog.debug("buffer %s: bind %d (was %d)\n",
                   getBufferName(bufferType).c_str(), id, prevId);
    }

    void Buffer::unbind()
    {
        gLog.debug("unbind %s\n", getBufferName(bufferType).c_str());

        GL_CHECK(glBindBuffer(bufferType, prevId));
        bufferType = 0;
        prevId = 0;
    }
} // namespace sb


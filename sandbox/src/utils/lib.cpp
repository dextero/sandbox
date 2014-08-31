#ifndef NO_CHECK_MACROS
#   define NO_CHECK_MACROS
#endif

#include "utils/lib.h"
#include "utils/logger.h"
#include "utils/debug.h"

#ifdef PLATFORM_WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#endif // PLATFORM_WIN32

#define TRACE_GL_CALLS 0
#include "rendering/includeGL.h"

#include <IL/ilu.h>

#define FAIL_ON_ERROR

namespace sb {

#if GL_CALL_QUEUE
Logger<QueueOutput> gl_queue_logger(QueueOutput(8));
#endif // GL_CALL_QUEUE

namespace utils {

// returns true on error
bool GLCheck(const char* file, int line, const char* call)
{
    GLuint err = glGetError();

    if (err != GL_NO_ERROR) {
#if GL_CALL_QUEUE
        gLog.debug("last GL calls:\n%s", 
                   ::sb::gl_queue_logger.getOutput().getOutput().c_str());
#endif
#ifdef FAIL_ON_ERROR
        sbFail(
#else
        gLog.err(
#endif
            "GL error: \"%s\" at file %s, line %d\n>> %s\n",
            gluErrorString(err), file, line, call);
    }

    return !!err;
}

// return true on error
bool ILCheck(const char* file, int line, const char* call)
{
    ILuint err = ilGetError();
    if (err != IL_NO_ERROR) {
#ifdef FAIL_ON_ERROR
        sbFail(
#else
        gLog.err(
#endif
            "DevIL error: \"%s\" at file %s, line %d\n>> %s\n", iluErrorString(err), file, line, call);
    }

    return !!err;
}

void print_integer(const char* name,
                   GLuint binding)
{
    GLuint id;
    glGetIntegerv(binding, (GLint*)&id);
    gLog.info("%s: %d\n", name, id);
}

void gl_debug()
{
    print_integer("ARRAY_BUFFER", GL_ARRAY_BUFFER_BINDING);
    print_integer("ELEMENT_ARRAY_BUFFER", GL_ELEMENT_ARRAY_BUFFER_BINDING);
    print_integer("TEXTURE_2D", GL_TEXTURE_BINDING_2D);
}

void checkGLAvailability() {
    gLog.info("checking GL functions availability\n");
    static struct GLFunc {
        const void* func;
        const char* name;
        const char* errMsg;
        enum {
            SevOptional,
            SevRequired
        } severity;
    } functions[] = {
#define FUNC_OPT(name, fail_msg) { (const void*)(name), #name, (fail_msg), GLFunc::SevOptional }
#define FUNC_REQ(name, fail_msg) { (const void*)(name), #name, (fail_msg), GLFunc::SevRequired }
        FUNC_REQ(glGenVertexArrays, 0),
        FUNC_REQ(glDeleteVertexArrays, 0),
        FUNC_REQ(glBindVertexArray, 0),
        FUNC_REQ(glGetVertexAttribiv, 0),
        FUNC_REQ(glVertexAttribPointer, 0),
        FUNC_REQ(glBindAttribLocation, 0),
        FUNC_REQ(glEnableVertexAttribArray, 0),
        FUNC_REQ(glDisableVertexAttribArray, 0),
        FUNC_REQ(glGenBuffers, 0),
        FUNC_REQ(glDeleteBuffers, 0),
        FUNC_REQ(glBindBuffer, 0),
        FUNC_REQ(glGetBufferParameteriv, 0),
        FUNC_REQ(glActiveTexture, 0),
        FUNC_REQ(glGenerateMipmap, 0),
        FUNC_REQ(glDrawElements, 0),
        FUNC_REQ(glUseProgram, 0),
        FUNC_REQ(glCreateProgram, 0),
        FUNC_REQ(glLinkProgram, 0),
        FUNC_REQ(glDeleteProgram, 0),
        FUNC_REQ(glCreateShader, 0),
        FUNC_REQ(glShaderSource, 0),
        FUNC_REQ(glCompileShader, 0),
        FUNC_REQ(glAttachShader, 0),
        FUNC_REQ(glDeleteShader, 0),
        FUNC_REQ(glDrawElements, 0),
        FUNC_REQ(glUniform1iv, 0),
        FUNC_REQ(glUniform1fv, 0),
        FUNC_REQ(glUniform2fv, 0),
        FUNC_REQ(glUniform3fv, 0),
        FUNC_REQ(glUniform4fv, 0),
        FUNC_REQ(glUniformMatrix4fv, 0)
#undef FUNC_OPT
#undef FUNC_REQ
    };

    uint32_t requiredFunctionsMissing = 0;
    for (size_t i = 0; i < sizeof(functions) / sizeof(functions[0]); ++i) {
        if (functions[i].func != NULL) {
            gLog.info("%-40sOK\n", functions[i].name);
        } else {
            if (functions[i].severity == GLFunc::SevOptional) {
                gLog.warn("%-40sNOT AVAILABLE\n", functions[i].name);
            } else {
                gLog.err("%-40sNOT AVAILABLE\n", functions[i].name);
                ++requiredFunctionsMissing;
            }

            if (functions[i].errMsg != NULL) {
                gLog.info(functions[i].errMsg);
            }
        }
    }

    if (requiredFunctionsMissing > 0) {
        sbFail("some critical GL functions missing, app would most likely crash\n");
    }
}

} // namespace utils
} // namespace gl


#ifndef LIBUTILS_H
#define LIBUTILS_H

#include <sandbox/utils/logger.h>

namespace sb
{
    namespace utils
    {
        void checkGLAvailability();

        // returns true on error
        bool GLCheck(const char* file, int line, const char* call);

        // return true on error
        bool ILCheck(const char* file, int line, const char* call);

        void gl_debug();
    }
}

#ifndef NO_CHECK_MACROS
#   if _DEBUG

#       define GL_CHECK(funccall)        (funccall), sb::utils::GLCheck(__FILE__, __LINE__, #funccall)
#       define IL_CHECK(funccall)        (funccall), sb::utils::ILCheck(__FILE__, __LINE__, #funccall)

#   else //NDEBUG

#       define GL_CHECK(funccall)        funccall, (glGetError() != GL_NO_ERROR)
#       define IL_CHECK(funccall)        funccall, (ilGetError() != IL_NO_ERROR)

#   endif //_DEBUG

    // returns retval on failure
#   define GL_CHECK_RET(funccall, retval)        if (GL_CHECK(funccall)) return retval
#   define IL_CHECK_RET(funccall, retval)        if (IL_CHECK(funccall)) return retval

#else //!NO_CHECK_MACROS

// compatibility
#   define GL_CHECK(funccall)                (funccall)
#   define IL_CHECK(funccall)                (funccall)
#   define GL_CHECK_RET(funccall, retval)    (funccall)
#   define IL_CHECK_RET(funccall, retval)    (funccall)

#endif

#endif //LIBUTILS_H

#ifndef LIBUTILS_H
#define LIBUTILS_H

namespace sb
{
    namespace utils
    {
        // returns true on error
        bool GLCheck(const char* file, int line, const char* call);

        // return true on error
        bool ILCheck(const char* file, int line, const char* call);
    }
}

#ifndef NO_CHECK_MACROS
#   ifdef _DEBUG

#       define GL_CHECK(funccall)        (funccall), sb::Utils::GLCheck(__FILE__, __LINE__, #funccall)
#       define IL_CHECK(funccall)        (funccall), sb::Utils::ILCheck(__FILE__, __LINE__, #funccall)

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

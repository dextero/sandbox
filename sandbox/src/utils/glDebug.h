#ifndef GLDEBUG_H
#define GLDEBUG_H

#include <utility>

#include "rendering/includeGL.h"
#include "utils/stringUtils.h"
#include "utils/logger.h"

namespace sb {

#if GL_CALL_QUEUE && !GL_CONTINIOUS_TRACE
extern Logger<QueueOutput> gl_queue_logger;
#   define glLog ::sb::gl_queue_logger
#else // GL_CALL_QUEUE
#   define glLog gLog
#endif // GL_CALL_QUEUE

const char* gl_const_to_string(GLulong id);

inline void print_arg(const char* str) { glLog.debug("  \"%s\"", str); }
inline void print_arg(const void* ptr) { glLog.debug("  %p", ptr); }

inline void print_arg(GLulong id)
{
    const char* glConstName = gl_const_to_string(id);

    if (glConstName) {
        glLog.debug("  %lu (0x%lx) = %s", id, id, glConstName);
    } else {
        glLog.debug("  %lu (0x%lx)", id, id, glConstName);
    }
}

inline void print_args() {}

template<typename First, typename... Rest>
inline void print_args(First&& first, Rest&&... rest)
{
    print_arg(std::forward<First>(first));
    print_args(std::forward<Rest>(rest)...);
}

template<typename RetT,
         typename... Args>
struct proxy_call_helper
{
    static inline RetT call(const char* func_name,
                            RetT (*func)(Args...),
                            Args... args)
    {
        glLog.debug("%s", func_name);
        print_args(args...);
        RetT ret = func(args...);
        glLog.debug("=> %s", lexical_cast<std::string>(ret).c_str());
        glLog.flush();
        return ret;
    }
};

template<typename... Args>
struct proxy_call_helper<void, Args...>
{
    static inline void call(const char* func_name,
                            void (*func)(Args...),
                            Args... args)
    {
        glLog.debug("%s", func_name);
        print_args(args...);
        func(args...);
        glLog.debug("=> void");
        glLog.flush();
    }
};

template<typename RetT,
         typename... Args,
         typename... ActualArgs>
inline RetT proxy_call(const char* func_name,
                       RetT (*func)(Args...),
                       ActualArgs... args)
{
    typedef proxy_call_helper<RetT, Args...> helper;
    return helper::call(func_name, func, static_cast<Args>(args)...);
}

template<typename DstT, typename... Args>
struct proxy_call_struct
{
    proxy_call_struct(const char* func_name,
                      DstT (*func)(Args...)):
        func_name(func_name),
        func(func)
    {}

    DstT operator ()(Args... args)
    {
        return proxy_call(func_name, func, args...);
    }

    operator const void*() const
    {
        return (const void*)func;
    }

    const char* func_name;
    DstT (*func)(Args...);
};

template<typename DstT, typename... Args>
inline proxy_call_struct<DstT, Args...>
make_proxy(const char *func_name,
           DstT (*func)(Args...))
{
    return proxy_call_struct<DstT, Args...>(func_name, func);
}

} // namespace sb

#undef GLEW_GET_FUN
#define GLEW_GET_FUN(fun) make_proxy(#fun, fun)

#define glClearIndex(...) sb::proxy_call("glClearIndex", glClearIndex, ##__VA_ARGS__)
#define glClearColor(...) sb::proxy_call("glClearColor", glClearColor, ##__VA_ARGS__)
#define glClear(...) sb::proxy_call("glClear", glClear, ##__VA_ARGS__)
#define glIndexMask(...) sb::proxy_call("glIndexMask", glIndexMask, ##__VA_ARGS__)
#define glColorMask(...) sb::proxy_call("glColorMask", glColorMask, ##__VA_ARGS__)
#define glAlphaFunc(...) sb::proxy_call("glAlphaFunc", glAlphaFunc, ##__VA_ARGS__)
#define glBlendFunc(...) sb::proxy_call("glBlendFunc", glBlendFunc, ##__VA_ARGS__)
#define glLogicOp(...) sb::proxy_call("glLogicOp", glLogicOp, ##__VA_ARGS__)
#define glCullFace(...) sb::proxy_call("glCullFace", glCullFace, ##__VA_ARGS__)
#define glFrontFace(...) sb::proxy_call("glFrontFace", glFrontFace, ##__VA_ARGS__)
#define glPointSize(...) sb::proxy_call("glPointSize", glPointSize, ##__VA_ARGS__)
#define glLineWidth(...) sb::proxy_call("glLineWidth", glLineWidth, ##__VA_ARGS__)
#define glLineStipple(...) sb::proxy_call("glLineStipple", glLineStipple, ##__VA_ARGS__)
#define glPolygonMode(...) sb::proxy_call("glPolygonMode", glPolygonMode, ##__VA_ARGS__)
#define glPolygonOffset(...) sb::proxy_call("glPolygonOffset", glPolygonOffset, ##__VA_ARGS__)
#define glPolygonStipple(...) sb::proxy_call("glPolygonStipple", glPolygonStipple, ##__VA_ARGS__)
#define glGetPolygonStipple(...) sb::proxy_call("glGetPolygonStipple", glGetPolygonStipple, ##__VA_ARGS__)
#define glEdgeFlag(...) sb::proxy_call("glEdgeFlag", glEdgeFlag, ##__VA_ARGS__)
#define glEdgeFlagv(...) sb::proxy_call("glEdgeFlagv", glEdgeFlagv, ##__VA_ARGS__)
#define glScissor(...) sb::proxy_call("glScissor", glScissor, ##__VA_ARGS__)
#define glClipPlane(...) sb::proxy_call("glClipPlane", glClipPlane, ##__VA_ARGS__)
#define glGetClipPlane(...) sb::proxy_call("glGetClipPlane", glGetClipPlane, ##__VA_ARGS__)
#define glDrawBuffer(...) sb::proxy_call("glDrawBuffer", glDrawBuffer, ##__VA_ARGS__)
#define glReadBuffer(...) sb::proxy_call("glReadBuffer", glReadBuffer, ##__VA_ARGS__)
#define glEnable(...) sb::proxy_call("glEnable", glEnable, ##__VA_ARGS__)
#define glDisable(...) sb::proxy_call("glDisable", glDisable, ##__VA_ARGS__)
#define glIsEnabled(...) sb::proxy_call("glIsEnabled", glIsEnabled, ##__VA_ARGS__)
#define glEnableClientState(...) sb::proxy_call("glEnableClientState", glEnableClientState, ##__VA_ARGS__)
#define glDisableClientState(...) sb::proxy_call("glDisableClientState", glDisableClientState, ##__VA_ARGS__)
#define glGetBooleanv(...) sb::proxy_call("glGetBooleanv", glGetBooleanv, ##__VA_ARGS__)
#define glGetDoublev(...) sb::proxy_call("glGetDoublev", glGetDoublev, ##__VA_ARGS__)
#define glGetFloatv(...) sb::proxy_call("glGetFloatv", glGetFloatv, ##__VA_ARGS__)
#define glGetIntegerv(...) sb::proxy_call("glGetIntegerv", glGetIntegerv, ##__VA_ARGS__)
#define glPushAttrib(...) sb::proxy_call("glPushAttrib", glPushAttrib, ##__VA_ARGS__)
//#define glPopAttrib(...) sb::proxy_call("glPopAttrib", glPopAttrib, ##__VA_ARGS__)
#define glPushClientAttrib(...) sb::proxy_call("glPushClientAttrib", glPushClientAttrib, ##__VA_ARGS__)
#define glPopClientAttrib(...) sb::proxy_call("glPopClientAttrib", glPopClientAttrib, ##__VA_ARGS__)
#define glRenderMode(...) sb::proxy_call("glRenderMode", glRenderMode, ##__VA_ARGS__)
//#define glGetError(...) sb::proxy_call("glGetError", glGetError, ##__VA_ARGS__)
#define glGetString(...) sb::proxy_call("glGetString", glGetString , ##__VA_ARGS__)
#define glFinish(...) sb::proxy_call("glFinish", glFinish, ##__VA_ARGS__)
#define glFlush(...) sb::proxy_call("glFlush", glFlush, ##__VA_ARGS__)
#define glHint(...) sb::proxy_call("glHint", glHint, ##__VA_ARGS__)
#define glClearDepth(...) sb::proxy_call("glClearDepth", glClearDepth, ##__VA_ARGS__)
#define glDepthFunc(...) sb::proxy_call("glDepthFunc", glDepthFunc, ##__VA_ARGS__)
#define glDepthMask(...) sb::proxy_call("glDepthMask", glDepthMask, ##__VA_ARGS__)
#define glDepthRange(...) sb::proxy_call("glDepthRange", glDepthRange, ##__VA_ARGS__)
#define glClearAccum(...) sb::proxy_call("glClearAccum", glClearAccum, ##__VA_ARGS__)
#define glAccum(...) sb::proxy_call("glAccum", glAccum, ##__VA_ARGS__)
#define glMatrixMode(...) sb::proxy_call("glMatrixMode", glMatrixMode, ##__VA_ARGS__)
#define glOrtho(...) sb::proxy_call("glOrtho", glOrtho, ##__VA_ARGS__)
#define glFrustum(...) sb::proxy_call("glFrustum", glFrustum, ##__VA_ARGS__)
#define glViewport(...) sb::proxy_call("glViewport", glViewport, ##__VA_ARGS__)
#define glPushMatrix(...) sb::proxy_call("glPushMatrix", glPushMatrix, ##__VA_ARGS__)
#define glPopMatrix(...) sb::proxy_call("glPopMatrix", glPopMatrix, ##__VA_ARGS__)
#define glLoadIdentity(...) sb::proxy_call("glLoadIdentity", glLoadIdentity, ##__VA_ARGS__)
#define glLoadMatrixd(...) sb::proxy_call("glLoadMatrixd", glLoadMatrixd, ##__VA_ARGS__)
#define glLoadMatrixf(...) sb::proxy_call("glLoadMatrixf", glLoadMatrixf, ##__VA_ARGS__)
#define glMultMatrixd(...) sb::proxy_call("glMultMatrixd", glMultMatrixd, ##__VA_ARGS__)
#define glMultMatrixf(...) sb::proxy_call("glMultMatrixf", glMultMatrixf, ##__VA_ARGS__)
#define glRotated(...) sb::proxy_call("glRotated", glRotated, ##__VA_ARGS__)
#define glRotatef(...) sb::proxy_call("glRotatef", glRotatef, ##__VA_ARGS__)
#define glScaled(...) sb::proxy_call("glScaled", glScaled, ##__VA_ARGS__)
#define glScalef(...) sb::proxy_call("glScalef", glScalef, ##__VA_ARGS__)
#define glTranslated(...) sb::proxy_call("glTranslated", glTranslated, ##__VA_ARGS__)
#define glTranslatef(...) sb::proxy_call("glTranslatef", glTranslatef, ##__VA_ARGS__)
#define glIsList(...) sb::proxy_call("glIsList", glIsList, ##__VA_ARGS__)
#define glDeleteLists(...) sb::proxy_call("glDeleteLists", glDeleteLists, ##__VA_ARGS__)
#define glGenLists(...) sb::proxy_call("glGenLists", glGenLists, ##__VA_ARGS__)
#define glNewList(...) sb::proxy_call("glNewList", glNewList, ##__VA_ARGS__)
#define glEndList(...) sb::proxy_call("glEndList", glEndList, ##__VA_ARGS__)
#define glCallList(...) sb::proxy_call("glCallList", glCallList, ##__VA_ARGS__)
#define glCallLists(...) sb::proxy_call("glCallLists", glCallLists, ##__VA_ARGS__)
#define glListBase(...) sb::proxy_call("glListBase", glListBase, ##__VA_ARGS__)
#define glBegin(...) sb::proxy_call("glBegin", glBegin, ##__VA_ARGS__)
#define glEnd(...) sb::proxy_call("glEnd", glEnd, ##__VA_ARGS__)
#define glVertex2d(...) sb::proxy_call("glVertex2d", glVertex2d, ##__VA_ARGS__)
#define glVertex2f(...) sb::proxy_call("glVertex2f", glVertex2f, ##__VA_ARGS__)
#define glVertex2i(...) sb::proxy_call("glVertex2i", glVertex2i, ##__VA_ARGS__)
#define glVertex2s(...) sb::proxy_call("glVertex2s", glVertex2s, ##__VA_ARGS__)
#define glVertex3d(...) sb::proxy_call("glVertex3d", glVertex3d, ##__VA_ARGS__)
#define glVertex3f(...) sb::proxy_call("glVertex3f", glVertex3f, ##__VA_ARGS__)
#define glVertex3i(...) sb::proxy_call("glVertex3i", glVertex3i, ##__VA_ARGS__)
#define glVertex3s(...) sb::proxy_call("glVertex3s", glVertex3s, ##__VA_ARGS__)
#define glVertex4d(...) sb::proxy_call("glVertex4d", glVertex4d, ##__VA_ARGS__)
#define glVertex4f(...) sb::proxy_call("glVertex4f", glVertex4f, ##__VA_ARGS__)
#define glVertex4i(...) sb::proxy_call("glVertex4i", glVertex4i, ##__VA_ARGS__)
#define glVertex4s(...) sb::proxy_call("glVertex4s", glVertex4s, ##__VA_ARGS__)
#define glVertex2dv(...) sb::proxy_call("glVertex2dv", glVertex2dv, ##__VA_ARGS__)
#define glVertex2fv(...) sb::proxy_call("glVertex2fv", glVertex2fv, ##__VA_ARGS__)
#define glVertex2iv(...) sb::proxy_call("glVertex2iv", glVertex2iv, ##__VA_ARGS__)
#define glVertex2sv(...) sb::proxy_call("glVertex2sv", glVertex2sv, ##__VA_ARGS__)
#define glVertex3dv(...) sb::proxy_call("glVertex3dv", glVertex3dv, ##__VA_ARGS__)
#define glVertex3fv(...) sb::proxy_call("glVertex3fv", glVertex3fv, ##__VA_ARGS__)
#define glVertex3iv(...) sb::proxy_call("glVertex3iv", glVertex3iv, ##__VA_ARGS__)
#define glVertex3sv(...) sb::proxy_call("glVertex3sv", glVertex3sv, ##__VA_ARGS__)
#define glVertex4dv(...) sb::proxy_call("glVertex4dv", glVertex4dv, ##__VA_ARGS__)
#define glVertex4fv(...) sb::proxy_call("glVertex4fv", glVertex4fv, ##__VA_ARGS__)
#define glVertex4iv(...) sb::proxy_call("glVertex4iv", glVertex4iv, ##__VA_ARGS__)
#define glVertex4sv(...) sb::proxy_call("glVertex4sv", glVertex4sv, ##__VA_ARGS__)
#define glNormal3b(...) sb::proxy_call("glNormal3b", glNormal3b, ##__VA_ARGS__)
#define glNormal3d(...) sb::proxy_call("glNormal3d", glNormal3d, ##__VA_ARGS__)
#define glNormal3f(...) sb::proxy_call("glNormal3f", glNormal3f, ##__VA_ARGS__)
#define glNormal3i(...) sb::proxy_call("glNormal3i", glNormal3i, ##__VA_ARGS__)
#define glNormal3s(...) sb::proxy_call("glNormal3s", glNormal3s, ##__VA_ARGS__)
#define glNormal3bv(...) sb::proxy_call("glNormal3bv", glNormal3bv, ##__VA_ARGS__)
#define glNormal3dv(...) sb::proxy_call("glNormal3dv", glNormal3dv, ##__VA_ARGS__)
#define glNormal3fv(...) sb::proxy_call("glNormal3fv", glNormal3fv, ##__VA_ARGS__)
#define glNormal3iv(...) sb::proxy_call("glNormal3iv", glNormal3iv, ##__VA_ARGS__)
#define glNormal3sv(...) sb::proxy_call("glNormal3sv", glNormal3sv, ##__VA_ARGS__)
#define glIndexd(...) sb::proxy_call("glIndexd", glIndexd, ##__VA_ARGS__)
#define glIndexf(...) sb::proxy_call("glIndexf", glIndexf, ##__VA_ARGS__)
#define glIndexi(...) sb::proxy_call("glIndexi", glIndexi, ##__VA_ARGS__)
#define glIndexs(...) sb::proxy_call("glIndexs", glIndexs, ##__VA_ARGS__)
#define glIndexub(...) sb::proxy_call("glIndexub", glIndexub, ##__VA_ARGS__)
#define glIndexdv(...) sb::proxy_call("glIndexdv", glIndexdv, ##__VA_ARGS__)
#define glIndexfv(...) sb::proxy_call("glIndexfv", glIndexfv, ##__VA_ARGS__)
#define glIndexiv(...) sb::proxy_call("glIndexiv", glIndexiv, ##__VA_ARGS__)
#define glIndexsv(...) sb::proxy_call("glIndexsv", glIndexsv, ##__VA_ARGS__)
#define glIndexubv(...) sb::proxy_call("glIndexubv", glIndexubv, ##__VA_ARGS__)
#define glColor3b(...) sb::proxy_call("glColor3b", glColor3b, ##__VA_ARGS__)
#define glColor3d(...) sb::proxy_call("glColor3d", glColor3d, ##__VA_ARGS__)
#define glColor3f(...) sb::proxy_call("glColor3f", glColor3f, ##__VA_ARGS__)
#define glColor3i(...) sb::proxy_call("glColor3i", glColor3i, ##__VA_ARGS__)
#define glColor3s(...) sb::proxy_call("glColor3s", glColor3s, ##__VA_ARGS__)
#define glColor3ub(...) sb::proxy_call("glColor3ub", glColor3ub, ##__VA_ARGS__)
#define glColor3ui(...) sb::proxy_call("glColor3ui", glColor3ui, ##__VA_ARGS__)
#define glColor3us(...) sb::proxy_call("glColor3us", glColor3us, ##__VA_ARGS__)
#define glColor4b(...) sb::proxy_call("glColor4b", glColor4b, ##__VA_ARGS__)
#define glColor4d(...) sb::proxy_call("glColor4d", glColor4d, ##__VA_ARGS__)
#define glColor4f(...) sb::proxy_call("glColor4f", glColor4f, ##__VA_ARGS__)
#define glColor4i(...) sb::proxy_call("glColor4i", glColor4i, ##__VA_ARGS__)
#define glColor4s(...) sb::proxy_call("glColor4s", glColor4s, ##__VA_ARGS__)
#define glColor4ub(...) sb::proxy_call("glColor4ub", glColor4ub, ##__VA_ARGS__)
#define glColor4ui(...) sb::proxy_call("glColor4ui", glColor4ui, ##__VA_ARGS__)
#define glColor4us(...) sb::proxy_call("glColor4us", glColor4us, ##__VA_ARGS__)
#define glColor3bv(...) sb::proxy_call("glColor3bv", glColor3bv, ##__VA_ARGS__)
#define glColor3dv(...) sb::proxy_call("glColor3dv", glColor3dv, ##__VA_ARGS__)
#define glColor3fv(...) sb::proxy_call("glColor3fv", glColor3fv, ##__VA_ARGS__)
#define glColor3iv(...) sb::proxy_call("glColor3iv", glColor3iv, ##__VA_ARGS__)
#define glColor3sv(...) sb::proxy_call("glColor3sv", glColor3sv, ##__VA_ARGS__)
#define glColor3ubv(...) sb::proxy_call("glColor3ubv", glColor3ubv, ##__VA_ARGS__)
#define glColor3uiv(...) sb::proxy_call("glColor3uiv", glColor3uiv, ##__VA_ARGS__)
#define glColor3usv(...) sb::proxy_call("glColor3usv", glColor3usv, ##__VA_ARGS__)
#define glColor4bv(...) sb::proxy_call("glColor4bv", glColor4bv, ##__VA_ARGS__)
#define glColor4dv(...) sb::proxy_call("glColor4dv", glColor4dv, ##__VA_ARGS__)
#define glColor4fv(...) sb::proxy_call("glColor4fv", glColor4fv, ##__VA_ARGS__)
#define glColor4iv(...) sb::proxy_call("glColor4iv", glColor4iv, ##__VA_ARGS__)
#define glColor4sv(...) sb::proxy_call("glColor4sv", glColor4sv, ##__VA_ARGS__)
#define glColor4ubv(...) sb::proxy_call("glColor4ubv", glColor4ubv, ##__VA_ARGS__)
#define glColor4uiv(...) sb::proxy_call("glColor4uiv", glColor4uiv, ##__VA_ARGS__)
#define glColor4usv(...) sb::proxy_call("glColor4usv", glColor4usv, ##__VA_ARGS__)
#define glTexCoord1d(...) sb::proxy_call("glTexCoord1d", glTexCoord1d, ##__VA_ARGS__)
#define glTexCoord1f(...) sb::proxy_call("glTexCoord1f", glTexCoord1f, ##__VA_ARGS__)
#define glTexCoord1i(...) sb::proxy_call("glTexCoord1i", glTexCoord1i, ##__VA_ARGS__)
#define glTexCoord1s(...) sb::proxy_call("glTexCoord1s", glTexCoord1s, ##__VA_ARGS__)
#define glTexCoord2d(...) sb::proxy_call("glTexCoord2d", glTexCoord2d, ##__VA_ARGS__)
#define glTexCoord2f(...) sb::proxy_call("glTexCoord2f", glTexCoord2f, ##__VA_ARGS__)
#define glTexCoord2i(...) sb::proxy_call("glTexCoord2i", glTexCoord2i, ##__VA_ARGS__)
#define glTexCoord2s(...) sb::proxy_call("glTexCoord2s", glTexCoord2s, ##__VA_ARGS__)
#define glTexCoord3d(...) sb::proxy_call("glTexCoord3d", glTexCoord3d, ##__VA_ARGS__)
#define glTexCoord3f(...) sb::proxy_call("glTexCoord3f", glTexCoord3f, ##__VA_ARGS__)
#define glTexCoord3i(...) sb::proxy_call("glTexCoord3i", glTexCoord3i, ##__VA_ARGS__)
#define glTexCoord3s(...) sb::proxy_call("glTexCoord3s", glTexCoord3s, ##__VA_ARGS__)
#define glTexCoord4d(...) sb::proxy_call("glTexCoord4d", glTexCoord4d, ##__VA_ARGS__)
#define glTexCoord4f(...) sb::proxy_call("glTexCoord4f", glTexCoord4f, ##__VA_ARGS__)
#define glTexCoord4i(...) sb::proxy_call("glTexCoord4i", glTexCoord4i, ##__VA_ARGS__)
#define glTexCoord4s(...) sb::proxy_call("glTexCoord4s", glTexCoord4s, ##__VA_ARGS__)
#define glTexCoord1dv(...) sb::proxy_call("glTexCoord1dv", glTexCoord1dv, ##__VA_ARGS__)
#define glTexCoord1fv(...) sb::proxy_call("glTexCoord1fv", glTexCoord1fv, ##__VA_ARGS__)
#define glTexCoord1iv(...) sb::proxy_call("glTexCoord1iv", glTexCoord1iv, ##__VA_ARGS__)
#define glTexCoord1sv(...) sb::proxy_call("glTexCoord1sv", glTexCoord1sv, ##__VA_ARGS__)
#define glTexCoord2dv(...) sb::proxy_call("glTexCoord2dv", glTexCoord2dv, ##__VA_ARGS__)
#define glTexCoord2fv(...) sb::proxy_call("glTexCoord2fv", glTexCoord2fv, ##__VA_ARGS__)
#define glTexCoord2iv(...) sb::proxy_call("glTexCoord2iv", glTexCoord2iv, ##__VA_ARGS__)
#define glTexCoord2sv(...) sb::proxy_call("glTexCoord2sv", glTexCoord2sv, ##__VA_ARGS__)
#define glTexCoord3dv(...) sb::proxy_call("glTexCoord3dv", glTexCoord3dv, ##__VA_ARGS__)
#define glTexCoord3fv(...) sb::proxy_call("glTexCoord3fv", glTexCoord3fv, ##__VA_ARGS__)
#define glTexCoord3iv(...) sb::proxy_call("glTexCoord3iv", glTexCoord3iv, ##__VA_ARGS__)
#define glTexCoord3sv(...) sb::proxy_call("glTexCoord3sv", glTexCoord3sv, ##__VA_ARGS__)
#define glTexCoord4dv(...) sb::proxy_call("glTexCoord4dv", glTexCoord4dv, ##__VA_ARGS__)
#define glTexCoord4fv(...) sb::proxy_call("glTexCoord4fv", glTexCoord4fv, ##__VA_ARGS__)
#define glTexCoord4iv(...) sb::proxy_call("glTexCoord4iv", glTexCoord4iv, ##__VA_ARGS__)
#define glTexCoord4sv(...) sb::proxy_call("glTexCoord4sv", glTexCoord4sv, ##__VA_ARGS__)
#define glRasterPos2d(...) sb::proxy_call("glRasterPos2d", glRasterPos2d, ##__VA_ARGS__)
#define glRasterPos2f(...) sb::proxy_call("glRasterPos2f", glRasterPos2f, ##__VA_ARGS__)
#define glRasterPos2i(...) sb::proxy_call("glRasterPos2i", glRasterPos2i, ##__VA_ARGS__)
#define glRasterPos2s(...) sb::proxy_call("glRasterPos2s", glRasterPos2s, ##__VA_ARGS__)
#define glRasterPos3d(...) sb::proxy_call("glRasterPos3d", glRasterPos3d, ##__VA_ARGS__)
#define glRasterPos3f(...) sb::proxy_call("glRasterPos3f", glRasterPos3f, ##__VA_ARGS__)
#define glRasterPos3i(...) sb::proxy_call("glRasterPos3i", glRasterPos3i, ##__VA_ARGS__)
#define glRasterPos3s(...) sb::proxy_call("glRasterPos3s", glRasterPos3s, ##__VA_ARGS__)
#define glRasterPos4d(...) sb::proxy_call("glRasterPos4d", glRasterPos4d, ##__VA_ARGS__)
#define glRasterPos4f(...) sb::proxy_call("glRasterPos4f", glRasterPos4f, ##__VA_ARGS__)
#define glRasterPos4i(...) sb::proxy_call("glRasterPos4i", glRasterPos4i, ##__VA_ARGS__)
#define glRasterPos4s(...) sb::proxy_call("glRasterPos4s", glRasterPos4s, ##__VA_ARGS__)
#define glRasterPos2dv(...) sb::proxy_call("glRasterPos2dv", glRasterPos2dv, ##__VA_ARGS__)
#define glRasterPos2fv(...) sb::proxy_call("glRasterPos2fv", glRasterPos2fv, ##__VA_ARGS__)
#define glRasterPos2iv(...) sb::proxy_call("glRasterPos2iv", glRasterPos2iv, ##__VA_ARGS__)
#define glRasterPos2sv(...) sb::proxy_call("glRasterPos2sv", glRasterPos2sv, ##__VA_ARGS__)
#define glRasterPos3dv(...) sb::proxy_call("glRasterPos3dv", glRasterPos3dv, ##__VA_ARGS__)
#define glRasterPos3fv(...) sb::proxy_call("glRasterPos3fv", glRasterPos3fv, ##__VA_ARGS__)
#define glRasterPos3iv(...) sb::proxy_call("glRasterPos3iv", glRasterPos3iv, ##__VA_ARGS__)
#define glRasterPos3sv(...) sb::proxy_call("glRasterPos3sv", glRasterPos3sv, ##__VA_ARGS__)
#define glRasterPos4dv(...) sb::proxy_call("glRasterPos4dv", glRasterPos4dv, ##__VA_ARGS__)
#define glRasterPos4fv(...) sb::proxy_call("glRasterPos4fv", glRasterPos4fv, ##__VA_ARGS__)
#define glRasterPos4iv(...) sb::proxy_call("glRasterPos4iv", glRasterPos4iv, ##__VA_ARGS__)
#define glRasterPos4sv(...) sb::proxy_call("glRasterPos4sv", glRasterPos4sv, ##__VA_ARGS__)
#define glRectd(...) sb::proxy_call("glRectd", glRectd, ##__VA_ARGS__)
#define glRectf(...) sb::proxy_call("glRectf", glRectf, ##__VA_ARGS__)
#define glRecti(...) sb::proxy_call("glRecti", glRecti, ##__VA_ARGS__)
#define glRects(...) sb::proxy_call("glRects", glRects, ##__VA_ARGS__)
#define glRectdv(...) sb::proxy_call("glRectdv", glRectdv, ##__VA_ARGS__)
#define glRectfv(...) sb::proxy_call("glRectfv", glRectfv, ##__VA_ARGS__)
#define glRectiv(...) sb::proxy_call("glRectiv", glRectiv, ##__VA_ARGS__)
#define glRectsv(...) sb::proxy_call("glRectsv", glRectsv, ##__VA_ARGS__)
#define glVertexPointer(...) sb::proxy_call("glVertexPointer", glVertexPointer, ##__VA_ARGS__)
#define glNormalPointer(...) sb::proxy_call("glNormalPointer", glNormalPointer, ##__VA_ARGS__)
#define glColorPointer(...) sb::proxy_call("glColorPointer", glColorPointer, ##__VA_ARGS__)
#define glIndexPointer(...) sb::proxy_call("glIndexPointer", glIndexPointer, ##__VA_ARGS__)
#define glTexCoordPointer(...) sb::proxy_call("glTexCoordPointer", glTexCoordPointer, ##__VA_ARGS__)
#define glEdgeFlagPointer(...) sb::proxy_call("glEdgeFlagPointer", glEdgeFlagPointer, ##__VA_ARGS__)
#define glGetPointerv(...) sb::proxy_call("glGetPointerv", glGetPointerv, ##__VA_ARGS__)
#define glArrayElement(...) sb::proxy_call("glArrayElement", glArrayElement, ##__VA_ARGS__)
#define glDrawArrays(...) sb::proxy_call("glDrawArrays", glDrawArrays, ##__VA_ARGS__)
#define glDrawElements(...) sb::proxy_call("glDrawElements", glDrawElements, ##__VA_ARGS__)
#define glInterleavedArrays(...) sb::proxy_call("glInterleavedArrays", glInterleavedArrays, ##__VA_ARGS__)
#define glShadeModel(...) sb::proxy_call("glShadeModel", glShadeModel, ##__VA_ARGS__)
#define glLightf(...) sb::proxy_call("glLightf", glLightf, ##__VA_ARGS__)
#define glLighti(...) sb::proxy_call("glLighti", glLighti, ##__VA_ARGS__)
#define glLightfv(...) sb::proxy_call("glLightfv", glLightfv, ##__VA_ARGS__)
#define glLightiv(...) sb::proxy_call("glLightiv", glLightiv, ##__VA_ARGS__)
#define glGetLightfv(...) sb::proxy_call("glGetLightfv", glGetLightfv, ##__VA_ARGS__)
#define glGetLightiv(...) sb::proxy_call("glGetLightiv", glGetLightiv, ##__VA_ARGS__)
#define glLightModelf(...) sb::proxy_call("glLightModelf", glLightModelf, ##__VA_ARGS__)
#define glLightModeli(...) sb::proxy_call("glLightModeli", glLightModeli, ##__VA_ARGS__)
#define glLightModelfv(...) sb::proxy_call("glLightModelfv", glLightModelfv, ##__VA_ARGS__)
#define glLightModeliv(...) sb::proxy_call("glLightModeliv", glLightModeliv, ##__VA_ARGS__)
#define glMaterialf(...) sb::proxy_call("glMaterialf", glMaterialf, ##__VA_ARGS__)
#define glMateriali(...) sb::proxy_call("glMateriali", glMateriali, ##__VA_ARGS__)
#define glMaterialfv(...) sb::proxy_call("glMaterialfv", glMaterialfv, ##__VA_ARGS__)
#define glMaterialiv(...) sb::proxy_call("glMaterialiv", glMaterialiv, ##__VA_ARGS__)
#define glGetMaterialfv(...) sb::proxy_call("glGetMaterialfv", glGetMaterialfv, ##__VA_ARGS__)
#define glGetMaterialiv(...) sb::proxy_call("glGetMaterialiv", glGetMaterialiv, ##__VA_ARGS__)
#define glColorMaterial(...) sb::proxy_call("glColorMaterial", glColorMaterial, ##__VA_ARGS__)
#define glPixelZoom(...) sb::proxy_call("glPixelZoom", glPixelZoom, ##__VA_ARGS__)
#define glPixelStoref(...) sb::proxy_call("glPixelStoref", glPixelStoref, ##__VA_ARGS__)
#define glPixelStorei(...) sb::proxy_call("glPixelStorei", glPixelStorei, ##__VA_ARGS__)
#define glPixelTransferf(...) sb::proxy_call("glPixelTransferf", glPixelTransferf, ##__VA_ARGS__)
#define glPixelTransferi(...) sb::proxy_call("glPixelTransferi", glPixelTransferi, ##__VA_ARGS__)
#define glPixelMapfv(...) sb::proxy_call("glPixelMapfv", glPixelMapfv, ##__VA_ARGS__)
#define glPixelMapuiv(...) sb::proxy_call("glPixelMapuiv", glPixelMapuiv, ##__VA_ARGS__)
#define glPixelMapusv(...) sb::proxy_call("glPixelMapusv", glPixelMapusv, ##__VA_ARGS__)
#define glGetPixelMapfv(...) sb::proxy_call("glGetPixelMapfv", glGetPixelMapfv, ##__VA_ARGS__)
#define glGetPixelMapuiv(...) sb::proxy_call("glGetPixelMapuiv", glGetPixelMapuiv, ##__VA_ARGS__)
#define glGetPixelMapusv(...) sb::proxy_call("glGetPixelMapusv", glGetPixelMapusv, ##__VA_ARGS__)
#define glBitmap(...) sb::proxy_call("glBitmap", glBitmap, ##__VA_ARGS__)
#define glReadPixels(...) sb::proxy_call("glReadPixels", glReadPixels, ##__VA_ARGS__)
#define glDrawPixels(...) sb::proxy_call("glDrawPixels", glDrawPixels, ##__VA_ARGS__)
#define glCopyPixels(...) sb::proxy_call("glCopyPixels", glCopyPixels, ##__VA_ARGS__)
#define glStencilFunc(...) sb::proxy_call("glStencilFunc", glStencilFunc, ##__VA_ARGS__)
#define glStencilMask(...) sb::proxy_call("glStencilMask", glStencilMask, ##__VA_ARGS__)
#define glStencilOp(...) sb::proxy_call("glStencilOp", glStencilOp, ##__VA_ARGS__)
#define glClearStencil(...) sb::proxy_call("glClearStencil", glClearStencil, ##__VA_ARGS__)
#define glTexGend(...) sb::proxy_call("glTexGend", glTexGend, ##__VA_ARGS__)
#define glTexGenf(...) sb::proxy_call("glTexGenf", glTexGenf, ##__VA_ARGS__)
#define glTexGeni(...) sb::proxy_call("glTexGeni", glTexGeni, ##__VA_ARGS__)
#define glTexGendv(...) sb::proxy_call("glTexGendv", glTexGendv, ##__VA_ARGS__)
#define glTexGenfv(...) sb::proxy_call("glTexGenfv", glTexGenfv, ##__VA_ARGS__)
#define glTexGeniv(...) sb::proxy_call("glTexGeniv", glTexGeniv, ##__VA_ARGS__)
#define glGetTexGendv(...) sb::proxy_call("glGetTexGendv", glGetTexGendv, ##__VA_ARGS__)
#define glGetTexGenfv(...) sb::proxy_call("glGetTexGenfv", glGetTexGenfv, ##__VA_ARGS__)
#define glGetTexGeniv(...) sb::proxy_call("glGetTexGeniv", glGetTexGeniv, ##__VA_ARGS__)
#define glTexEnvf(...) sb::proxy_call("glTexEnvf", glTexEnvf, ##__VA_ARGS__)
#define glTexEnvi(...) sb::proxy_call("glTexEnvi", glTexEnvi, ##__VA_ARGS__)
#define glTexEnvfv(...) sb::proxy_call("glTexEnvfv", glTexEnvfv, ##__VA_ARGS__)
#define glTexEnviv(...) sb::proxy_call("glTexEnviv", glTexEnviv, ##__VA_ARGS__)
#define glGetTexEnvfv(...) sb::proxy_call("glGetTexEnvfv", glGetTexEnvfv, ##__VA_ARGS__)
#define glGetTexEnviv(...) sb::proxy_call("glGetTexEnviv", glGetTexEnviv, ##__VA_ARGS__)
#define glTexParameterf(...) sb::proxy_call("glTexParameterf", glTexParameterf, ##__VA_ARGS__)
#define glTexParameteri(...) sb::proxy_call("glTexParameteri", glTexParameteri, ##__VA_ARGS__)
#define glTexParameterfv(...) sb::proxy_call("glTexParameterfv", glTexParameterfv, ##__VA_ARGS__)
#define glTexParameteriv(...) sb::proxy_call("glTexParameteriv", glTexParameteriv, ##__VA_ARGS__)
#define glGetTexParameterfv(...) sb::proxy_call("glGetTexParameterfv", glGetTexParameterfv, ##__VA_ARGS__)
#define glGetTexParameteriv(...) sb::proxy_call("glGetTexParameteriv", glGetTexParameteriv, ##__VA_ARGS__)
#define glGetTexLevelParameterfv(...) sb::proxy_call("glGetTexLevelParameterfv", glGetTexLevelParameterfv, ##__VA_ARGS__)
#define glGetTexLevelParameteriv(...) sb::proxy_call("glGetTexLevelParameteriv", glGetTexLevelParameteriv, ##__VA_ARGS__)
#define glTexImage1D(...) sb::proxy_call("glTexImage1D", glTexImage1D, ##__VA_ARGS__)
#define glTexImage2D(...) sb::proxy_call("glTexImage2D", glTexImage2D, ##__VA_ARGS__)
#define glGetTexImage(...) sb::proxy_call("glGetTexImage", glGetTexImage, ##__VA_ARGS__)
#define glGenTextures(...) sb::proxy_call("glGenTextures", glGenTextures, ##__VA_ARGS__)
#define glDeleteTextures(...) sb::proxy_call("glDeleteTextures", glDeleteTextures, ##__VA_ARGS__)
#define glBindTexture(...) sb::proxy_call("glBindTexture", glBindTexture, ##__VA_ARGS__)
#define glPrioritizeTextures(...) sb::proxy_call("glPrioritizeTextures", glPrioritizeTextures, ##__VA_ARGS__)
#define glAreTexturesResident(...) sb::proxy_call("glAreTexturesResident", glAreTexturesResident, ##__VA_ARGS__)
#define glIsTexture(...) sb::proxy_call("glIsTexture", glIsTexture, ##__VA_ARGS__)
#define glTexSubImage1D(...) sb::proxy_call("glTexSubImage1D", glTexSubImage1D, ##__VA_ARGS__)
#define glTexSubImage2D(...) sb::proxy_call("glTexSubImage2D", glTexSubImage2D, ##__VA_ARGS__)
#define glCopyTexImage1D(...) sb::proxy_call("glCopyTexImage1D", glCopyTexImage1D, ##__VA_ARGS__)
#define glCopyTexImage2D(...) sb::proxy_call("glCopyTexImage2D", glCopyTexImage2D, ##__VA_ARGS__)
#define glCopyTexSubImage1D(...) sb::proxy_call("glCopyTexSubImage1D", glCopyTexSubImage1D, ##__VA_ARGS__)
#define glCopyTexSubImage2D(...) sb::proxy_call("glCopyTexSubImage2D", glCopyTexSubImage2D, ##__VA_ARGS__)
#define glMap1d(...) sb::proxy_call("glMap1d", glMap1d, ##__VA_ARGS__)
#define glMap1f(...) sb::proxy_call("glMap1f", glMap1f, ##__VA_ARGS__)
#define glMap2d(...) sb::proxy_call("glMap2d", glMap2d, ##__VA_ARGS__)
#define glMap2f(...) sb::proxy_call("glMap2f", glMap2f, ##__VA_ARGS__)
#define glGetMapdv(...) sb::proxy_call("glGetMapdv", glGetMapdv, ##__VA_ARGS__)
#define glGetMapfv(...) sb::proxy_call("glGetMapfv", glGetMapfv, ##__VA_ARGS__)
#define glGetMapiv(...) sb::proxy_call("glGetMapiv", glGetMapiv, ##__VA_ARGS__)
#define glEvalCoord1d(...) sb::proxy_call("glEvalCoord1d", glEvalCoord1d, ##__VA_ARGS__)
#define glEvalCoord1f(...) sb::proxy_call("glEvalCoord1f", glEvalCoord1f, ##__VA_ARGS__)
#define glEvalCoord1dv(...) sb::proxy_call("glEvalCoord1dv", glEvalCoord1dv, ##__VA_ARGS__)
#define glEvalCoord1fv(...) sb::proxy_call("glEvalCoord1fv", glEvalCoord1fv, ##__VA_ARGS__)
#define glEvalCoord2d(...) sb::proxy_call("glEvalCoord2d", glEvalCoord2d, ##__VA_ARGS__)
#define glEvalCoord2f(...) sb::proxy_call("glEvalCoord2f", glEvalCoord2f, ##__VA_ARGS__)
#define glEvalCoord2dv(...) sb::proxy_call("glEvalCoord2dv", glEvalCoord2dv, ##__VA_ARGS__)
#define glEvalCoord2fv(...) sb::proxy_call("glEvalCoord2fv", glEvalCoord2fv, ##__VA_ARGS__)
#define glMapGrid1d(...) sb::proxy_call("glMapGrid1d", glMapGrid1d, ##__VA_ARGS__)
#define glMapGrid1f(...) sb::proxy_call("glMapGrid1f", glMapGrid1f, ##__VA_ARGS__)
#define glMapGrid2d(...) sb::proxy_call("glMapGrid2d", glMapGrid2d, ##__VA_ARGS__)
#define glMapGrid2f(...) sb::proxy_call("glMapGrid2f", glMapGrid2f, ##__VA_ARGS__)
#define glEvalPoint1(...) sb::proxy_call("glEvalPoint1", glEvalPoint1, ##__VA_ARGS__)
#define glEvalPoint2(...) sb::proxy_call("glEvalPoint2", glEvalPoint2, ##__VA_ARGS__)
#define glEvalMesh1(...) sb::proxy_call("glEvalMesh1", glEvalMesh1, ##__VA_ARGS__)
#define glEvalMesh2(...) sb::proxy_call("glEvalMesh2", glEvalMesh2, ##__VA_ARGS__)
#define glFogf(...) sb::proxy_call("glFogf", glFogf, ##__VA_ARGS__)
#define glFogi(...) sb::proxy_call("glFogi", glFogi, ##__VA_ARGS__)
#define glFogfv(...) sb::proxy_call("glFogfv", glFogfv, ##__VA_ARGS__)
#define glFogiv(...) sb::proxy_call("glFogiv", glFogiv, ##__VA_ARGS__)
#define glFeedbackBuffer(...) sb::proxy_call("glFeedbackBuffer", glFeedbackBuffer, ##__VA_ARGS__)
#define glPassThrough(...) sb::proxy_call("glPassThrough", glPassThrough, ##__VA_ARGS__)
#define glSelectBuffer(...) sb::proxy_call("glSelectBuffer", glSelectBuffer, ##__VA_ARGS__)
#define glInitNames(...) sb::proxy_call("glInitNames", glInitNames, ##__VA_ARGS__)
#define glLoadName(...) sb::proxy_call("glLoadName", glLoadName, ##__VA_ARGS__)
#define glPushName(...) sb::proxy_call("glPushName", glPushName, ##__VA_ARGS__)
#define glPopName(...) sb::proxy_call("glPopName", glPopName, ##__VA_ARGS__)

#endif // GLDEBUG_H


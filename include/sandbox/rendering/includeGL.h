#ifndef INCLUDEGL_H
#define INCLUDEGL_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#ifndef GL_TRACE_CALLS
#   define GL_TRACE_CALLS 1
#endif // TRACE_GL_CALLS
#ifndef GL_CONTINIOUS_TRACE
#   define GL_CONTINIOUS_TRACE 0
#endif // GL_CONTINIOUS_TRACE

#if _DEBUG && GL_TRACE_CALLS
#   include <sandbox/utils/glDebug.h>
#endif // _DEBUG && TRACE_GL_CALLS

#endif // INCLUDEGL_H


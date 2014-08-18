#ifndef INCLUDEGL_H
#define INCLUDEGL_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#ifndef TRACE_GL_CALLS
#   define TRACE_GL_CALLS 0
#endif // TRACE_GL_CALLS

#if _DEBUG && TRACE_GL_CALLS
#   include "utils/glDebug.h"
#endif // _DEBUG && TRACE_GL_CALLS

#endif // INCLUDEGL_H


#ifndef RENDERER_TYPES_H
#define RENDERER_TYPES_H

#ifdef PLATFORM_WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#endif // PLATFORM_WIN32

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

namespace sb
{
    typedef GLuint IdType;
    typedef IdType TextureId;
    typedef IdType BufferId;
    typedef IdType ProgramId;
    typedef IdType ShaderId;

#if defined(_WIN64) || defined(_LP64) || defined(__LP64__)
    typedef unsigned long long ResourceHandle;
#else // 32bit
    typedef IdType ResourceHandle;
#endif

    enum EProjectionType {
        ProjectionOrthographic,
        ProjectionPerspective
    };
} // namespace sb

#define SHAPE_POINTS                GL_POINTS
#define SHAPE_LINES                 GL_LINES
#define SHAPE_TRIANGLES             GL_TRIANGLES
#define SHAPE_QUADS                 GL_QUADS
#define SHAPE_TRIANGLE_STRIP        GL_TRIANGLE_STRIP

#define RENDERER_DEPTH_TEST         GL_DEPTH_TEST
#define RENDERER_BACKFACE_CULLING   GL_CULL_FACE
#define RENDERER_ALPHA_BLENDING     GL_BLEND

#endif //RENDERER_TYPES_H


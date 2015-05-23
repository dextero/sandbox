#ifndef STRING_H
#define STRING_H

#include <sandbox/rendering/color.h>
#include <sandbox/rendering/types.h>
#include <sandbox/utils/types.h>

#include <string>
#include <GL/glx.h>

namespace sb
{
#if 0
    class String
    {
        static GLuint mBase;
        static ::Display* mDisplay;
        static int mFontHeight;
        static float mLineHeight;

        static void printLine(const std::string& str,
                              float x,
                              float y,
                              const Color& color);
    public:
        static void init(::Display* display);
        static void release();
        static void recalculateLineHeight();

        // (0,0) - top-left corner, (1,1) - bottom-right corner
        static void print(const std::string& str,
                          float x,
                          float y,
                          const Color& color = Color(1.f, 1.f, 1.f),
                          uint32_t line = 0u);
    };
    #endif
} // namespace sb

#endif // STRING_H

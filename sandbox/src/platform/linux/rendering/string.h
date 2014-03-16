#ifndef STRING_H
#define STRING_H

#include "rendering/color.h"
#include "rendering/types.h"
#include "utils/types.h"

#include <string>
#include <GL/glx.h>

namespace sb
{
    class String
    {
        static GLuint mBase;
        static ::Display* mDisplay;
        static int mFontHeight;
        static float mLineHeight;

        static void PrintLine(const std::string& str, float x, float y, const Color& color);
    public:
        static void Init(::Display* display);
        static void Release();
        static void RecalculateLineHeight();

        // (0,0) - top-left corner, (1,1) - bottom-right corner
        static void Print(const std::string& str, float x, float y, const Color& color = Color(1.f, 1.f, 1.f), uint line = 0u);
    };
} // namespace sb

#endif // STRING_H

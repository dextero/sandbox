#ifndef STRING_H
#define STRING_H

#include "color.h"
#include "types.h"
#include "utils/types.h"
#include <string>

namespace sb
{
    class String
    {
        static GLuint mBase;
        static HDC mDC;
        static int mFontHeight;
        static float mLineHeight;

        static void PrintLine(const std::string& str, float x, float y, const Color& color);
    public:
        static void Init(HDC dc);
        static void Release();
        static void RecalculateLineHeight();

        // (0,0) - top-left corner, (1,1) - bottom-right corner
        static void Print(const std::string& str, float x, float y, const Color& color = Color(1.f, 1.f, 1.f), uint32_t line = 0u);
    };
} // namespace sb

#endif // STRING_H

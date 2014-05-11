#include "rendering/string.h"
#include "rendering/shader.h"
#include "utils/libUtils.h"
#include "utils/stringUtils.h"
#include "utils/logger.h"

namespace sb
{
    GLuint String::mBase = 0u;
    HDC String::mDC = 0;
    int String::mFontHeight = 14;
    float String::mLineHeight = 0.f;


    void String::PrintLine(const std::string& str, float x, float y, const Color& color)
    {
        Vec3 pos(x, y, 0.f);
        GL_CHECK(glRasterPos3fv((GLfloat*)&pos));

        GL_CHECK(glPushAttrib(GL_LIST_BIT));
        GL_CHECK(glListBase(mBase - 32));
        GL_CHECK(glCallLists(str.size(), GL_UNSIGNED_BYTE, str.c_str()));
        GL_CHECK(glPopAttrib());
    }

    void String::Init(HDC dc)
    {
        gLog.Info("initializing font bitmaps...\n");

        HFONT font, oldfont;
        mDC = dc;
        mBase = GL_CHECK(glGenLists(96));
        font = CreateFont(-mFontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, (LPCTSTR)"Courier New");
        oldfont = (HFONT)::SelectObject(dc, font);
        wglUseFontBitmaps(dc, 32, 96, mBase);
        SelectObject(dc, oldfont);
        DeleteObject(font);
    }

    void String::Release()
    {
        GL_CHECK(glDeleteLists(mBase, 96));
    }

    void String::RecalculateLineHeight()
    {
        HWND wnd = ::WindowFromDC(mDC);
        RECT rect;
        ::GetClientRect(wnd, &rect);

        mLineHeight = 2.f / (float)((rect.bottom - rect.top) / mFontHeight);
    }

    // (0,0) - top-left corner, (1,1) - bottom-right corner
    void String::Print(const std::string& str, float x, float y, const Color& color, uint32_t line)
    {
        GL_CHECK(glDisable(GL_TEXTURE_2D));

        x = (x - 0.5f) * 2.f;
        y = (y - 0.5f) * -2.f - 0.03f;

        Shader::Use(Shader::ShaderNone);

        GL_CHECK(glColor4fv((GLfloat*)&color));

        std::vector<std::string> lines = StringUtils::Split(str, '\n');
        if (lines.size() > 1)
        {
            RecalculateLineHeight();

            for (size_t i = 0; i < lines.size(); ++i)
                PrintLine(lines[i], x, y - (float)(i + line) * mLineHeight, color);
        }
        else
            PrintLine(lines[0], x, y - (float)line * mLineHeight, color);
    }
} // namespace sb

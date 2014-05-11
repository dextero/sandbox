#include "rendering/string.h"
#include "rendering/shader.h"
#include "utils/libUtils.h"
#include "utils/stringUtils.h"
#include "utils/profiler.h"
#include "utils/logger.h"

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

namespace sb
{
    GLuint String::mBase = 0u;
    ::Display* String::mDisplay = 0;
    int String::mFontHeight = 14;
    float String::mLineHeight = 0.f;


    void String::PrintLine(const std::string& str,
                           float x,
                           float y,
                           const Color& /*color*/)
    {
        Vec3 pos(x, y, 0.f);
        GL_CHECK(glRasterPos3fv((GLfloat*)&pos));

        GL_CHECK(glPushAttrib(GL_LIST_BIT));
        GL_CHECK(glListBase(mBase - 32));
        GL_CHECK(glCallLists(str.size(), GL_UNSIGNED_BYTE, str.c_str()));
        GL_CHECK(glPopAttrib());
    }

    void String::Init(::Display* display)
    {
        gLog.Info("initializing font bitmaps...\n");

        mDisplay = display;

        XFontStruct* font;
        mBase = GL_CHECK(glGenLists(96));
        font = XLoadQueryFont(mDisplay, "-*-courier new-bold-r-normal--14-*-*-*-p-*-iso8895-2");
        if (font == NULL)
        {
            gLog.Warn("using fixed font\n");
            font = XLoadQueryFont(mDisplay, "fixed");

            if (font == NULL)
                gLog.Err("couldn't load bitmap font!\n");
        }

        glXUseXFont(font->fid, 32, 96, mBase);
        XFreeFont(mDisplay, font);
    }

    void String::Release()
    {
        GL_CHECK(glDeleteLists(mBase, 96));
    }

    void String::RecalculateLineHeight()
    {
        ::Window window;
        int focusState;
        XGetInputFocus(mDisplay, &window, &focusState);

        XWindowAttributes attribs;
        XGetWindowAttributes(mDisplay, window, &attribs);

        mLineHeight = 2.f / ((float)(attribs.height) / mFontHeight);
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

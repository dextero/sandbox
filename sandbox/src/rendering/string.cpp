#include <sandbox/rendering/includeGL.h>
#include <sandbox/rendering/string.h>
#include <sandbox/rendering/shader.h>
#include <sandbox/utils/lib.h>
#include <sandbox/utils/stringUtils.h>
#include <sandbox/utils/logger.h>

#include <X11/Xlib.h>

#if 0
namespace sb
{
    GLuint String::mBase = 0u;
    ::Display* String::mDisplay = 0;
    int String::mFontHeight = 14;
    float String::mLineHeight = 0.f;


    void String::printLine(const std::string& str,
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

    void String::init(::Display* display)
    {
        gLog.trace("initializing font bitmaps...\n");

        mDisplay = display;

        XFontStruct* font;
        mBase = GL_CHECK(glGenLists(96));
        font = XLoadQueryFont(mDisplay, "-*-courier new-bold-r-normal--14-*-*-*-p-*-iso8895-2");
        if (font == NULL)
        {
            gLog.warn("using fixed font\n");
            font = XLoadQueryFont(mDisplay, "fixed");

            if (font == NULL)
                gLog.err("couldn't load bitmap font!\n");
        }

        glXUseXFont(font->fid, 32, 96, mBase);
        XFreeFont(mDisplay, font);
    }

    void String::release()
    {
        GL_CHECK(glDeleteLists(mBase, 96));
    }

    void String::recalculateLineHeight()
    {
        ::Window window;
        int focusState;
        XGetInputFocus(mDisplay, &window, &focusState);

        XWindowAttributes attribs;
        XGetWindowAttributes(mDisplay, window, &attribs);

        mLineHeight = 2.f / ((float)(attribs.height) / mFontHeight);
    }

    // (0,0) - top-left corner, (1,1) - bottom-right corner
    void String::print(const std::string& str, float x, float y, const Color& color, uint32_t line)
    {
        GL_CHECK(glDisable(GL_TEXTURE_2D));

        x = (x - 0.5f) * 2.f;
        y = (y - 0.5f) * -2.f - 0.03f;

        GL_CHECK(glColor4fv((GLfloat*)&color));

        std::vector<std::string> lines = utils::split(str, '\n');
        if (lines.size() > 1)
        {
            recalculateLineHeight();

            for (size_t i = 0; i < lines.size(); ++i)
                printLine(lines[i], x, y - (float)(i + line) * mLineHeight, color);
        }
        else
            printLine(lines[0], x, y - (float)line * mLineHeight, color);
    }
} // namespace sb
#endif

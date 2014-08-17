#include "window.h"

#include "utils/stringUtils.h"
#include "utils/logger.h"
#include "utils/debug.h"

#include <X11/Xutil.h>
#include <cstring>

namespace sb
{
    Window::Window(unsigned width, unsigned height):
        mDisplay(NULL),
        mWindow(0),
        mLockCursor(false),
        mFullscreen(false),
        mRenderer(),
        mEvents()

    {
        create(width, height);
        mRenderer.setViewport(0, 0, width, height);
    }

    Window::~Window()
    {
        close();
    }


    bool Window::create(unsigned width, unsigned height)
    {
        mDisplay = XOpenDisplay(0);
        if (!mDisplay)
            return false;

        static int visualAttribs[] = {
            GLX_X_RENDERABLE,  True,
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
            GLX_RENDER_TYPE,   GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
            GLX_RED_SIZE,      8,
            GLX_GREEN_SIZE,    8,
            GLX_BLUE_SIZE,     8,
            GLX_ALPHA_SIZE,    8,
            GLX_DEPTH_SIZE,    24,
            GLX_STENCIL_SIZE,  8,
            GLX_DOUBLEBUFFER,  True,
            None
        };

        gLog.trace("getting framebuffer config\n");
        int fbCount;
        GLXFBConfig* fbc = glXChooseFBConfig(mDisplay, DefaultScreen(mDisplay), visualAttribs, &fbCount);

        int bestFbcId = -1, worstFbcId = -1, bestNumSamp = -1, worstNumSamp = 999;
        for (int i = 0; i < fbCount; ++i)
        {
            XVisualInfo* vi = glXGetVisualFromFBConfig(mDisplay, fbc[i]);
            if (vi)
            {
                int sampBuf, samples;
                glXGetFBConfigAttrib(mDisplay, fbc[i], GLX_SAMPLE_BUFFERS, &sampBuf);
                glXGetFBConfigAttrib(mDisplay, fbc[i], GLX_SAMPLES, &samples);

                gLog.trace("matching fbconfig %d, visual id 0x%2x: SAMPLE_BUFFERS = %d, SAMPLES = %d\n", i, vi->visualid, sampBuf, samples);

                if (bestFbcId < 0 || (sampBuf && samples > bestNumSamp))
                    bestFbcId = i, bestNumSamp = samples;
                if (worstFbcId < 0 || !sampBuf || samples < worstNumSamp)
                    worstFbcId = i, worstNumSamp = samples;
            }
            XFree(vi);
        }

        GLXFBConfig bestFbc = fbc[bestFbcId];

        XFree(fbc);
        XVisualInfo* vi = glXGetVisualFromFBConfig(mDisplay, bestFbc);
        gLog.trace("chosen visual id = 0x%x\n", vi->visualid);

        gLog.trace("creating colormap\n");
        XSetWindowAttributes swa;
        Colormap cmap;
        swa.colormap = cmap = XCreateColormap(mDisplay, RootWindow(mDisplay, vi->screen), vi->visual, AllocNone);
        swa.background_pixmap = None;
        swa.border_pixel = 0;
        swa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

        gLog.trace("creating mask\n");
        mWindow = XCreateWindow(mDisplay, RootWindow(mDisplay, vi->screen), 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);

        if (!mWindow)
        {
            gLog.err("failed to create window\n");
            return false;
        }

        XFree(vi);
        XStoreName(mDisplay, mWindow, "Window");

        gLog.trace("mapping window\n");
        XMapWindow(mDisplay, mWindow);

        mRenderer.init(mDisplay, mWindow, bestFbc);

        return true;
    }

    void Window::resize(unsigned width, unsigned height)
    {
        (void)width;
        (void)height;
        sbFail("Window::resize not implemented");
    }

    bool Window::setFullscreen(bool fullscreen)
    {
        sbFail("Window::setFullscreen not implemented");

        mFullscreen = fullscreen;
        return true;
    }

    const Vec2i Window::getSize()
    {
        XWindowAttributes attribs;
        XGetWindowAttributes(mDisplay, mWindow, &attribs);

        return Vec2i(attribs.width, attribs.height);
    }

    void Window::close()
    {
        if (mDisplay && mWindow)
        {
            XDestroyWindow(mDisplay, mWindow);
            mWindow = 0;
        }
    }

    bool Window::getEvent(Event& e)
    {
        while (XPending(mDisplay))
        {
            XEvent event;
            XNextEvent(mDisplay, &event);

            switch (event.type)
            {
            case KeyPress:
                mEvents.push(Event::keyPressedEvent((Key::Code)XLookupKeysym((XKeyEvent*)&event, 0)));
                break;
            case KeyRelease:
                mEvents.push(Event::keyReleasedEvent((Key::Code)XLookupKeysym((XKeyEvent*)&event, 0)));
                break;
            case ButtonPress:
                if (event.xbutton.button < Button4)
                    mEvents.push(Event::mousePressedEvent(event.xbutton.x, event.xbutton.y, (Mouse::Button)event.xbutton.button));
                else
                    mEvents.push(Event::mouseWheelEvent(event.xbutton.x, event.xbutton.y, (Mouse::Button)event.xbutton.button == Button4 ? -1 : 1));
                break;
            case ButtonRelease:
                mEvents.push(Event::mousePressedEvent(event.xbutton.x, event.xbutton.y, (Mouse::Button)event.xbutton.button));
                break;
            case MotionNotify:
                mEvents.push(Event::mouseMovedEvent(event.xmotion.x, event.xmotion.y));

                if (mLockCursor)
                {
                    static bool ignore = false;
                    if (!ignore)
                    {
                        Vec2i wndSize = getSize();
                        XWarpPointer(mDisplay, None, mWindow, 0, 0, 0, 0, wndSize.x / 2, wndSize.y / 2);
                    }
                    ignore = !ignore;
                }
                break;
            case FocusIn:
                mEvents.push(Event::windowFocusEvent(true));
                break;
            case FocusOut:
                mEvents.push(Event::windowFocusEvent(false));
                break;
            case DestroyNotify:
                mEvents.push(Event::windowClosedEvent());
                break;
            default:
                break;
            }
        }

        if (mEvents.size() > 0)
        {
            e = mEvents.front();
            mEvents.pop();
            return true;
        }
        else
            return false;
    }

    bool Window::isOpened()
    {
        return mWindow != 0;
    }

    bool Window::hasFocus()
    {
        ::Window focused;
        int focusState;
        XGetInputFocus(mDisplay, &focused, &focusState);
        return mWindow == focused;
    }

    void Window::setTitle(const std::string& str)
    {
        XStoreName(mDisplay, mWindow, str.c_str());
    }


    void Window::clear(const Color& c)
    {
        mRenderer.setClearColor(c);
        mRenderer.clear();
    }

    void Window::draw(Drawable& d)
    {
        mRenderer.draw(d);
    }

    void Window::display()
    {
        mRenderer.drawAll();
        glXSwapBuffers(mDisplay, mWindow);
    }

    void Window::hideCursor(bool hide)
    {
        if (hide)
        {
            char bm[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            Pixmap pix = XCreateBitmapFromData(mDisplay, mWindow, bm, 1, 1);
            XColor black;
            memset(&black, 0, sizeof(XColor));
            black.flags = DoRed | DoGreen | DoBlue;
            Cursor pointer = XCreatePixmapCursor(mDisplay, pix, pix, &black, &black, 0, 0);
            XFreePixmap(mDisplay, pix);

            XDefineCursor(mDisplay, mWindow, pointer);
            XSync(mDisplay, False); /* again, optional */
        }
        else
            XDefineCursor(mDisplay, mWindow, 0);
    }

    void Window::lockCursor(bool lock)
    {
        mLockCursor = lock;
    }

    void Window::saveScreenshot(const std::string& filename)
    {
        Vec2i size = getSize();
        mRenderer.saveScreenshot(filename, size.x, size.y);
    }


    Renderer& Window::getRenderer()
    {
        return mRenderer;
    }

    Camera& Window::getCamera()
    {
        return mRenderer.getCamera();
    }

} // namespace sb

#include "window.h"

#include "utils/stringUtils.h"
#include "utils/logger.h"

#include <X11/Xutil.h>
#include <cstring>

namespace sb
{
    Window::Window(unsigned width, unsigned height):
        mDisplay(NULL),
        mWindow(0),
        mLockCursor(false),
        mFullscreen(false)
    {
        Create(width, height);
        mRenderer.SetViewport(0, 0, width, height);
    }

    Window::~Window()
    {
        Close();
    }


    bool Window::Create(unsigned width, unsigned height)
    {
        mDisplay = XOpenDisplay(0);
        if (!mDisplay)
            return false;

        static int visualAttribs[] = {
            GLX_X_RENDERABLE,    True,
            GLX_DRAWABLE_TYPE,    GLX_WINDOW_BIT,
            GLX_RENDER_TYPE,    GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE,    GLX_TRUE_COLOR,
            GLX_RED_SIZE,        8,
            GLX_GREEN_SIZE,        8,
            GLX_BLUE_SIZE,        8,
            GLX_ALPHA_SIZE,        8,
            GLX_DEPTH_SIZE,        24,
            GLX_STENCIL_SIZE,    8,
            GLX_DOUBLEBUFFER,    True,
            None
        };

        gLog.Info("getting framebuffer config\n");
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

                gLog.Info("matching fbconfig %d, visual id 0x%2x: SAMPLE_BUFFERS = %d, SAMPLES = %d\n", i, vi->visualid, sampBuf, samples);

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
        gLog.Info("chosen visual id = 0x%x\n", vi->visualid);

        gLog.Info("creating colormap\n");
        XSetWindowAttributes swa;
        Colormap cmap;
        swa.colormap = cmap = XCreateColormap(mDisplay, RootWindow(mDisplay, vi->screen), vi->visual, AllocNone);
        swa.background_pixmap = None;
        swa.border_pixel = 0;
        swa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

        gLog.Info("creating mask\n");
        mWindow = XCreateWindow(mDisplay, RootWindow(mDisplay, vi->screen), 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);

        if (!mWindow)
        {
            gLog.Info("failed to create window\n");
            return false;
        }

        XFree(vi);
        XStoreName(mDisplay, mWindow, "Window");

        gLog.Info("mapping window\n");
        XMapWindow(mDisplay, mWindow);

        mRenderer.Init(mDisplay, mWindow, bestFbc);

        return true;
    }

    void Window::Resize(unsigned width, unsigned height)
    {
        (void)width;
        (void)height;
        assert(0);
    }

    bool Window::SetFullscreen(bool fullscreen)
    {
        (void)fullscreen;
        assert(!"Window::SetFullscreen not implemented!");

        return true;
    }

    const Vec2i Window::GetSize()
    {
        XWindowAttributes attribs;
        XGetWindowAttributes(mDisplay, mWindow, &attribs);

        return Vec2i(attribs.width, attribs.height);
    }

    void Window::Close()
    {
        if (mDisplay && mWindow)
        {
            XDestroyWindow(mDisplay, mWindow);
            mWindow = 0;
        }
    }

    bool Window::GetEvent(Event& e)
    {
        while (XPending(mDisplay))
        {
            XEvent event;
            XNextEvent(mDisplay, &event);

            switch (event.type)
            {
            case KeyPress:
                mEvents.push(Event::KeyPressedEvent((Key::Code)XLookupKeysym((XKeyEvent*)&event, 0)));
                break;
            case KeyRelease:
                mEvents.push(Event::KeyReleasedEvent((Key::Code)XLookupKeysym((XKeyEvent*)&event, 0)));
                break;
            case ButtonPress:
                if (event.xbutton.button < Button4)
                    mEvents.push(Event::MousePressedEvent(event.xbutton.x, event.xbutton.y, (Mouse::Button)event.xbutton.button));
                else
                    mEvents.push(Event::MouseWheelEvent(event.xbutton.x, event.xbutton.y, (Mouse::Button)event.xbutton.button == Button4 ? -1 : 1));
                break;
            case ButtonRelease:
                mEvents.push(Event::MousePressedEvent(event.xbutton.x, event.xbutton.y, (Mouse::Button)event.xbutton.button));
                break;
            case MotionNotify:
                mEvents.push(Event::MouseMovedEvent(event.xmotion.x, event.xmotion.y));

                if (mLockCursor)
                {
                    static bool ignore = false;
                    if (!ignore)
                    {
                        Vec2i wndSize = GetSize();
                        XWarpPointer(mDisplay, None, mWindow, 0, 0, 0, 0, wndSize[0] / 2, wndSize[1] / 2);
                    }
                    ignore = !ignore;
                }
                break;
            case FocusIn:
                mEvents.push(Event::WindowFocusEvent(true));
                break;
            case FocusOut:
                mEvents.push(Event::WindowFocusEvent(false));
                break;
            case DestroyNotify:
                mEvents.push(Event::WindowClosedEvent());
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

    bool Window::IsOpened()
    {
        return mWindow != 0;
    }

    bool Window::HasFocus()
    {
        ::Window focused;
        int focusState;
        XGetInputFocus(mDisplay, &focused, &focusState);
        return mWindow == focused;
    }

    void Window::SetTitle(const std::string& str)
    {
        XStoreName(mDisplay, mWindow, str.c_str());
    }

    void Window::SetTitle(const std::wstring& str)
    {
        XStoreName(mDisplay, mWindow, StringUtils::ToString(str).c_str());
    }


    void Window::Clear(const Color& c)
    {
        mRenderer.SetClearColor(c);
        mRenderer.Clear();
    }

    void Window::Draw(Drawable& d)
    {
        mRenderer.Draw(d);
    }

    void Window::Display()
    {
        mRenderer.DrawAll();
        glXSwapBuffers(mDisplay, mWindow);
    }

    void Window::HideCursor(bool hide)
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

    void Window::LockCursor(bool lock)
    {
        mLockCursor = lock;
    }

    void Window::SaveScreenshot(const wchar_t* filename)
    {
        Vec2i size = GetSize();
        mRenderer.SaveScreenshot(filename, size[0], size[1]);
    }


    Renderer& Window::GetRenderer()
    {
        return mRenderer;
    }

    Camera& Window::GetCamera()
    {
        return mRenderer.mCamera;
    }

} // namespace sb

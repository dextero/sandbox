#include "window.h"

#include "utils/stringUtils.h"
#include "utils/logger.h"

namespace sb
{
    LRESULT __stdcall Window::wndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l)
    {
        Window* wndPtr = (Window*)::GetWindowLongPtrA(hwnd, 0);

        switch (msg)
        {
        case WM_KEYDOWN:
            if (wndPtr)
            {
                wndPtr->mEvents.push(Event::keyPressedEvent((Key::Code)w));
                if (w == Key::Shift)
                    // stupid windows
                    wndPtr->mEvents.push(Event::keyPressedEvent((::GetKeyState(Key::LShift) >> 4) ? Key::LShift : Key::RShift));
            }
            break;
        case WM_KEYUP:
            if (wndPtr)
                wndPtr->mEvents.push(Event::keyReleasedEvent((Key::Code)w));
            break;
        case WM_LBUTTONDOWN:
            if (wndPtr)
                wndPtr->mEvents.push(Event::mousePressedEvent(LOWORD(l), HIWORD(l), Mouse::ButtonLeft));
            break;
        case WM_LBUTTONUP:
            if (wndPtr)
                wndPtr->mEvents.push(Event::mouseReleasedEvent(LOWORD(l), HIWORD(l), Mouse::ButtonLeft));
            break;
        case WM_RBUTTONDOWN:
            if (wndPtr)
                wndPtr->mEvents.push(Event::mousePressedEvent(LOWORD(l), HIWORD(l), Mouse::ButtonRight));
            break;
        case WM_RBUTTONUP:
            if (wndPtr)
                wndPtr->mEvents.push(Event::mouseReleasedEvent(LOWORD(l), HIWORD(l), Mouse::ButtonRight));
            break;
        case WM_MBUTTONDOWN:
            if (wndPtr)
                wndPtr->mEvents.push(Event::mousePressedEvent(LOWORD(l), HIWORD(l), Mouse::ButtonMiddle));
            break;
        case WM_MBUTTONUP:
            if (wndPtr)
                wndPtr->mEvents.push(Event::mouseReleasedEvent(LOWORD(l), HIWORD(l), Mouse::ButtonMiddle));
            break;
        case WM_XBUTTONDOWN:
            if (wndPtr && (w & 0x60))
                wndPtr->mEvents.push(Event::mousePressedEvent(LOWORD(l), HIWORD(l), w & 0x20 ? Mouse::ButtonX1 : Mouse::ButtonX2));
            break;
        case WM_XBUTTONUP:
            if (wndPtr && (w & 0x60))
                wndPtr->mEvents.push(Event::mouseReleasedEvent(LOWORD(l), HIWORD(l), w & 0x20 ? Mouse::ButtonX1 : Mouse::ButtonX2));
            break;
        case WM_MOUSEWHEEL:
            if (wndPtr)
                wndPtr->mEvents.push(Event::mouseWheelEvent(LOWORD(l), HIWORD(l), HIWORD(w) * WHEEL_DELTA));
            break;
        case WM_MOUSEMOVE:
            {
                static bool ignore = false;
                if (ignore)
                    ignore = false;
                else if (wndPtr)
                {
                    wndPtr->mEvents.push(Event::mouseMovedEvent(LOWORD(l), HIWORD(l)));
                    if (wndPtr->mLockCursor)
                    {
                        RECT rect;
                        POINT center;
                        ::GetClientRect(wndPtr->mWnd, &rect);
                        center.x = (rect.left + rect.right) / 2;
                        center.y = (rect.top + rect.bottom) / 2;
                        ::ClientToScreen(wndPtr->mWnd, &center);
                        ::SetCursorPos(center.x, center.y);
                        ignore = true;
                    }
                }
                break;
            }
        case WM_ACTIVATEAPP:
            if (wndPtr)
                wndPtr->mEvents.push(Event::windowFocusEvent(!!w));
            break;
        case WM_SIZE:
            if ((w == SIZE_RESTORED || w == SIZE_MAXIMIZED) && wndPtr)
            {
                Vec2i size = wndPtr->getSize();
                wndPtr->mEvents.push(Event::windowResizedEvent(size[0], size[1]));
                wndPtr->mRenderer.setViewport(0, 0, size[0], size[1]);
            }
            break;
        case WM_CREATE:
            ::SetWindowLongPtrA(hwnd, 0, (LONG)((CREATESTRUCT*)l)->lpCreateParams);
            break;
        case WM_CLOSE:
            if (wndPtr)
                wndPtr->mEvents.push(Event::windowClosedEvent());
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            break;
        default:
            return ::DefWindowProcA(hwnd, msg, w, l);
        }

        return 0;
    }

    Window::Window(unsigned width, unsigned height):
        mInstance(NULL),
        mWnd(NULL),
        mLockCursor(false),
        mFullscreen(false)
    {
        mInstance = ::GetModuleHandleA(0);

        create(width, height);
        mRenderer.init(mWnd);
        mRenderer.setViewport(0, 0, width, height);
    }


    bool Window::create(unsigned width, unsigned height)
    {
        static bool classRegistered = false;
        static LPCTSTR className = _T("ILikeTrains8@3");

        if (!classRegistered)
        {
            WNDCLASSEXW wcex;
            memset(&wcex, 0, sizeof(wcex));

            wcex.cbSize = sizeof(wcex);
            wcex.hInstance = ::GetModuleHandle(0);
            wcex.lpfnWndProc = WndProc;
            wcex.lpszClassName = className;
            wcex.cbWndExtra = 8;
            wcex.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;

            classRegistered = !!RegisterClassExW(&wcex);
            if (!classRegistered)
                return false;
        }

        RECT rect;
        rect.left = rect.top = 0;
        rect.right = width;
        rect.bottom = height;
        ::AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, NULL);
        mWnd = ::CreateWindowExW(NULL, className, _T("Window"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, mInstance, (void*)this);

        PIXELFORMATDESCRIPTOR pfd = {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
            PFD_TYPE_RGBA,
            32,
            0, 0, 0, 0, 0, 0,
            0, 0, 0,
            0, 0, 0, 0,
            16,
            0, 0,
            PFD_MAIN_PLANE,
            0, 0, 0, 0
        };

        HDC dc = ::GetDC(mWnd);
        int pixelFormat = ::ChoosePixelFormat(dc, &pfd);
        if (!pixelFormat)
            return false;

        if (!::SetPixelFormat(dc, pixelFormat, &pfd))
            return false;

        ::ShowWindow(mWnd, SW_SHOW);
        ::UpdateWindow(mWnd);

        return true;
    }

    void Window::resize(unsigned width, unsigned height)
    {
        assert(mWnd);

        RECT rect;
        rect.left = rect.top = 0;
        rect.right = width;
        rect.bottom = height;
        ::AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, NULL);
        ::SetWindowPos(mWnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
        mRenderer.setViewport(0, 0, width, height);
    }

    bool Window::setFullscreen(bool fullscreen)
    {
        assert(!"Window::setFullscreen not implemented!");

        if (fullscreen != mFullscreen)
        {
            if (fullscreen)
            {
                Vec2i size = getSize();

                DEVMODE settings;
                ::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &settings);
                settings.dmPelsWidth = size[0];
                settings.dmPelsHeight = size[1];
                settings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

                const char* error;
                switch(::ChangeDisplaySettings(&settings, CDS_FULLSCREEN))
                {
                case DISP_CHANGE_SUCCESSFUL:
                    return true;
                case DISP_CHANGE_BADDUALVIEW:
                    error = "the system is DualView capable"; break;
                case DISP_CHANGE_BADFLAGS:
                    error = "invalid flags"; break;
                case DISP_CHANGE_BADMODE:
                    error = "graphics mode not supported"; break;
                case DISP_CHANGE_BADPARAM:
                    error = "invalid parameter passed in"; break;
                case DISP_CHANGE_FAILED:
                    error = "display driver fai"; break;
                case DISP_CHANGE_NOTUPDATED:
                    error = "unable to write settings to the registry"; break;
                case DISP_CHANGE_RESTART:
                    error = "the computer needs to be restarted"; break;
                default:
                    error = "unknown error"; break;
                }

                gLog.err("couldn't set fullscreen mode (%s)\n", error);
                return false;
            }
            else
                ::ChangeDisplaySettings(NULL, 0);

            mFullscreen = fullscreen;
        }

        return true;
    }

    const Vec2i Window::getSize()
    {
        RECT rect;
        ::GetClientRect(mWnd, &rect);

        return Vec2i(rect.right - rect.left, rect.bottom - rect.top);
    }

    void Window::close()
    {
        ::DestroyWindow(mWnd);
        mWnd = NULL;
    }

    bool Window::getEvent(Event& e)
    {
        MSG msg;
        while (PeekMessage(&msg, mWnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        size_t eventsPending = mEvents.size();
        if (eventsPending)
        {
            e = mEvents.front();
            mEvents.pop();
        }

        return eventsPending > 0;
    }

    bool Window::isOpened()
    {
        return mWnd != NULL;
    }

    bool Window::hasFocus()
    {
        return ::GetActiveWindow() == mWnd;
    }

    void Window::setTitle(const std::string& str)
    {
#ifdef UNICODE
        ::SetWindowText(mWnd, utils::toWString(str).c_str());
#else // !UNICODE
        ::SetWindowText(mWnd, str.c_str());
#endif // UNICODE
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
        ::SwapBuffers(::GetDC(mWnd));
    }

    void Window::hideCursor(bool hide)
    {
        ::ShowCursor(!hide);
    }

    void Window::lockCursor(bool lock)
    {
        mLockCursor = lock;
    }

    void Window::saveScreenshot(const char* filename)
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
        return mRenderer.mCamera;
    }
} // namespace sb

#include "window.h"

#include "utils/profiler.h"
#include "utils/stringUtils.h"
#include "utils/logger.h"

namespace sb
{
    LRESULT __stdcall Window::WndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l)
    {
        PROFILE();

        Window* wndPtr = (Window*)::GetWindowLongPtrA(hwnd, 0);

        switch (msg)
        {
        case WM_KEYDOWN:
            if (wndPtr)
            {
                wndPtr->mEvents.push(Event::KeyPressedEvent((Key::Code)w));
                if (w == Key::Shift)
                    // stupid windows
                    wndPtr->mEvents.push(Event::KeyPressedEvent((::GetKeyState(Key::LShift) >> 4) ? Key::LShift : Key::RShift));
            }
            break;
        case WM_KEYUP:
            if (wndPtr)
                wndPtr->mEvents.push(Event::KeyReleasedEvent((Key::Code)w));
            break;
        case WM_LBUTTONDOWN:
            if (wndPtr)
                wndPtr->mEvents.push(Event::MousePressedEvent(LOWORD(l), HIWORD(l), Mouse::ButtonLeft));
            break;
        case WM_LBUTTONUP:
            if (wndPtr)
                wndPtr->mEvents.push(Event::MouseReleasedEvent(LOWORD(l), HIWORD(l), Mouse::ButtonLeft));
            break;
        case WM_RBUTTONDOWN:
            if (wndPtr)
                wndPtr->mEvents.push(Event::MousePressedEvent(LOWORD(l), HIWORD(l), Mouse::ButtonRight));
            break;
        case WM_RBUTTONUP:
            if (wndPtr)
                wndPtr->mEvents.push(Event::MouseReleasedEvent(LOWORD(l), HIWORD(l), Mouse::ButtonRight));
            break;
        case WM_MBUTTONDOWN:
            if (wndPtr)
                wndPtr->mEvents.push(Event::MousePressedEvent(LOWORD(l), HIWORD(l), Mouse::ButtonMiddle));
            break;
        case WM_MBUTTONUP:
            if (wndPtr)
                wndPtr->mEvents.push(Event::MouseReleasedEvent(LOWORD(l), HIWORD(l), Mouse::ButtonMiddle));
            break;
        case WM_XBUTTONDOWN:
            if (wndPtr && (w & 0x60))
                wndPtr->mEvents.push(Event::MousePressedEvent(LOWORD(l), HIWORD(l), w & 0x20 ? Mouse::ButtonX1 : Mouse::ButtonX2));
            break;
        case WM_XBUTTONUP:
            if (wndPtr && (w & 0x60))
                wndPtr->mEvents.push(Event::MouseReleasedEvent(LOWORD(l), HIWORD(l), w & 0x20 ? Mouse::ButtonX1 : Mouse::ButtonX2));
            break;
        case WM_MOUSEWHEEL:
            if (wndPtr)
                wndPtr->mEvents.push(Event::MouseWheelEvent(LOWORD(l), HIWORD(l), HIWORD(w) * WHEEL_DELTA));
            break;
        case WM_MOUSEMOVE:
            {
                static bool ignore = false;
                if (ignore)
                    ignore = false;
                else if (wndPtr)
                {
                    wndPtr->mEvents.push(Event::MouseMovedEvent(LOWORD(l), HIWORD(l)));
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
                wndPtr->mEvents.push(Event::WindowFocusEvent(!!w));
            break;
        case WM_SIZE:
            if ((w == SIZE_RESTORED || w == SIZE_MAXIMIZED) && wndPtr)
            {
                Vec2i size = wndPtr->GetSize();
                wndPtr->mEvents.push(Event::WindowResizedEvent(size[0], size[1]));
                wndPtr->mRenderer.SetViewport(0, 0, size[0], size[1]);
            }
            break;
        case WM_CREATE:
            ::SetWindowLongPtrA(hwnd, 0, (LONG)((CREATESTRUCT*)l)->lpCreateParams);
            break;
        case WM_CLOSE:
            if (wndPtr)
                wndPtr->mEvents.push(Event::WindowClosedEvent());
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
        PROFILE();

        mInstance = ::GetModuleHandleA(0);

        Create(width, height);
        mRenderer.Init(mWnd);
        mRenderer.SetViewport(0, 0, width, height);
    }

    Window::~Window()
    {
        PROFILE();
    }


    bool Window::Create(unsigned width, unsigned height)
    {
        PROFILE();

        static bool classRegistered = false;
        static const wchar_t* className = L"ILikeTrains8@3";

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
        mWnd = ::CreateWindowExW(NULL, className, (LPCTSTR)"Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, mInstance, (void*)this);

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

    void Window::Resize(unsigned width, unsigned height)
    {
        PROFILE();

        assert(mWnd);

        RECT rect;
        rect.left = rect.top = 0;
        rect.right = width;
        rect.bottom = height;
        ::AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, NULL);
        ::SetWindowPos(mWnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
        mRenderer.SetViewport(0, 0, width, height);
    }

    bool Window::SetFullscreen(bool fullscreen)
    {
        PROFILE();

        assert(!"Window::SetFullscreen not implemented!");

        if (fullscreen != mFullscreen)
        {
            if (fullscreen)
            {
                Vec2i size = GetSize();

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

                gLog.Err("couldn't set fullscreen mode (%s)\n", error);
                return false;
            }
            else
                ::ChangeDisplaySettings(NULL, 0);

            mFullscreen = fullscreen;
        }

        return true;
    }

    const Vec2i Window::GetSize()
    {
        RECT rect;
        ::GetClientRect(mWnd, &rect);

        return Vec2i(rect.right - rect.left, rect.bottom - rect.top);
    }

    void Window::Close()
    {
        PROFILE();

        ::DestroyWindow(mWnd);
        mWnd = NULL;
    }

    bool Window::GetEvent(Event& e)
    {
        PROFILE();

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

    bool Window::IsOpened()
    {
        PROFILE();

        return mWnd != NULL;
    }

    bool Window::HasFocus()
    {
        PROFILE();

        return ::GetActiveWindow() == mWnd;
    }

    void Window::SetTitle(const std::string& str)
    {
        PROFILE();

        ::SetWindowText(mWnd, (LPCTSTR)str.c_str());
    }

    void Window::SetTitle(const std::wstring& str)
    {
        PROFILE();

        ::SetWindowText(mWnd, (LPCTSTR)StringUtils::ToWString(str).c_str());
    }


    void Window::Clear(const Color& c)
    {
        PROFILE();

        mRenderer.SetClearColor(c);
        mRenderer.Clear();
    }

    void Window::Draw(Drawable& d)
    {
        PROFILE();

        mRenderer.Draw(d);
    }

    void Window::Display()
    {
        PROFILE();

        mRenderer.DrawAll();
        ::SwapBuffers(::GetDC(mWnd));
    }

    void Window::HideCursor(bool hide)
    {
        ::ShowCursor(!hide);
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

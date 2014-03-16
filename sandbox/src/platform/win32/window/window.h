#ifndef WINDOW_H
#define WINDOW_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <queue>

#include "rendering/renderer.h"
#include "rendering/color.h"
#include "rendering/camera.h"
#include "window/event.h"
#include "utils/allocator.h"


namespace sb
{
    class Window: OWN_ALLOC
    {
    private:
        HINSTANCE mInstance;
        HWND mWnd;

        bool mLockCursor, mFullscreen;

        Renderer mRenderer;
        std::queue<Event> mEvents;

        static LRESULT __stdcall WndProc(HWND, UINT, WPARAM, LPARAM);
    public:
        Window(unsigned width, unsigned height);
        ~Window();

        bool Create(unsigned width, unsigned height);
        void Resize(unsigned width, unsigned height);
        bool SetFullscreen(bool fullscreen = true);
        const Vec2i GetSize();
        void Close();
        bool GetEvent(Event& e);
        bool IsOpened();
        bool HasFocus();
        void SetTitle(const std::string& str);
        void SetTitle(const std::wstring& str);

        void Clear(const Color& c);
        void Draw(Drawable& d);
        void Display();
        void HideCursor(bool hide = true);
        void LockCursor(bool lock = true);
        void SaveScreenshot(const wchar_t* filename);

        Renderer& GetRenderer();
        Camera& GetCamera();
    };
} // namespace sb

#endif //WINDOW_H

#ifndef WINDOW_H
#define WINDOW_H

#include <queue>

#include <X11/Xlib.h>

#include "rendering/renderer.h"
#include "rendering/color.h"
#include "rendering/camera.h"
#include "window/event.h"


namespace sb
{
    class Window
    {
    private:
        ::Display* mDisplay;
        ::Window mWindow;

        bool mLockCursor, mFullscreen;

        Renderer mRenderer;
        std::queue<Event> mEvents;

    public:
        Window(unsigned width, unsigned height);
        ~Window();

        bool create(unsigned width, unsigned height);
        void resize(unsigned width, unsigned height);
        bool setFullscreen(bool fullscreen = true);
        const Vec2i getSize();
        void close();
        bool getEvent(Event& e);
        bool isOpened();
        bool hasFocus();
        void setTitle(const std::string& str);

        void clear(const Color& c);
        void draw(Drawable& d);
        void display();
        void hideCursor(bool hide = true);
        void lockCursor(bool lock = true);
        void saveScreenshot(const std::string& filename);

        Renderer& getRenderer();
        Camera& getCamera();
    };
} // namespace sb

#endif //WINDOW_H

#ifndef WINDOW_H
#define WINDOW_H

#include <queue>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "rendering/renderer.h"
#include "rendering/color.h"
#include "rendering/camera.h"
#include "window/event.h"


namespace sb
{
    class Window
    {
    public:
        Window(unsigned width, unsigned height);
        ~Window();

        Window(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator =(const Window&) = delete;
        Window& operator =(Window&&) = delete;

        bool create(unsigned width, unsigned height);
        void resize(unsigned width, unsigned height);
        bool setFullscreen(bool fullscreen = true);
        Vec2i getSize() const;
        void close();
        bool getEvent(Event& e);
        bool isOpened() const;
        bool isFullscreen() const;
        bool hasFocus() const;
        void setTitle(const std::string& str);

        void clear(const Color& c);
        void setAmbientLightColor(const Color& color) { mRenderer.setAmbientLightColor(color); }
        void addLight(const Light& light) { mRenderer.addLight(light); }
        void draw(Drawable& d);
        void display();
        void hideCursor(bool hide = true);
        void lockCursor(bool lock = true);
        void saveScreenshot(const std::string& filename);

        Renderer& getRenderer();
        Camera& getCamera();

        void drawString(const std::string& str,
                        const Vec2& topLeft = Vec2(0.0f, 0.0f),
                        const Color& color = sb::Color::White,
                        uint32_t lineNum = 0);

    private:
        ::Display* mDisplay;
        ::Window mWindow;

        bool mLockCursor; bool mFullscreen;
        Vec2i mSize;

        Renderer mRenderer;
        std::queue<Event> mEvents;

        template <typename T>
        int setXProperty(const char* propName, T value) {
            return XChangeProperty(mDisplay,
                                   mWindow,
                                   XInternAtom(mDisplay, propName, False),
                                   XA_ATOM,
                                   32,
                                   PropModeReplace,
                                   (unsigned char*)&value, sizeof(value) / sizeof(long));
        }
    };
} // namespace sb

#endif //WINDOW_H

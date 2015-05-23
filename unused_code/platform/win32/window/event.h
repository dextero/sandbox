#ifndef EVENT_H
#define EVENT_H

#include <WinUser.h>


namespace sb
{
    namespace Mouse
    {
        enum Button {
            ButtonNone = 0x0,
            ButtonLeft = VK_LBUTTON,
            ButtonRight = VK_RBUTTON,
            ButtonMiddle = VK_MBUTTON,
            ButtonX1 = VK_XBUTTON1,
            ButtonX2 = VK_XBUTTON2
        };
    }

    namespace Key
    {
        enum Code {
            Backspace = VK_BACK,
            Esc = VK_ESCAPE,
            Enter = VK_RETURN,
            CapsLock = VK_CAPITAL,
            ScrollLock = VK_SCROLL,
            NumLock = VK_NUMLOCK,
            Delete = VK_DELETE,
            Insert = VK_INSERT,
            Pause = VK_PAUSE,
            PrintScreen = VK_SNAPSHOT,
            Sleep = VK_SLEEP,
            Space = VK_SPACE,
            PageUp = VK_PRIOR,
            PageDown = VK_NEXT,
            Home = VK_HOME,
            End = VK_END,

            Comma = VK_OEM_COMMA,        /* ,< */
            Period = VK_OEM_PERIOD,        /* .> */
            Equal = VK_OEM_PLUS,            /* += */
            Minus = VK_OEM_MINUS,        /* -_ */
            Colon = VK_OEM_1,            /* ;: */
            Slash = VK_OEM_2,            /* /? */
            Tilde = VK_OEM_3,            /* `~ */
            LBracket = VK_OEM_4,        /* [{ */
            Backslash = VK_OEM_5,        /* \| */
            RBracket = VK_OEM_6,        /* ]} */
            Apostrophe = VK_OEM_7,        /* '" */

            LCtrl = VK_LCONTROL,
            LAlt = VK_LMENU,
            LShift = VK_LSHIFT,
            LWin = VK_LWIN,
            RCtrl = VK_RCONTROL,
            RAlt = VK_RMENU,
            RShift = VK_RSHIFT,
            RWin = VK_RWIN,
            Shift = VK_SHIFT,            /* LShift or RShift */

            ArrowDown = VK_DOWN,
            ArrowRight = VK_RIGHT,
            ArrowLeft = VK_LEFT,
            ArrowUp = VK_UP,

            F1 = VK_F1,
            F2 = VK_F2,
            F3 = VK_F3,
            F4 = VK_F4,
            F5 = VK_F5,
            F6 = VK_F6,
            F7 = VK_F7,
            F8 = VK_F8,
            F9 = VK_F9,
            F10 = VK_F10,
            F11 = VK_F11,
            F12 = VK_F12,

            Numpad0 = VK_NUMPAD0,
            Numpad1 = VK_NUMPAD1,
            Numpad2 = VK_NUMPAD2,
            Numpad3 = VK_NUMPAD3,
            Numpad4 = VK_NUMPAD4,
            Numpad5 = VK_NUMPAD5,
            Numpad6 = VK_NUMPAD6,
            Numpad7 = VK_NUMPAD7,
            Numpad8 = VK_NUMPAD8,
            Numpad9 = VK_NUMPAD9,
            NumpadDecimal = VK_DECIMAL,
            NumpadMultiply = VK_MULTIPLY,
            NumpadDivide = VK_DIVIDE,
            NumpadSubtract = VK_SUBTRACT,
            NumpadAdd = VK_ADD,

            Num0 = '0',
            Num1 = '1',
            Num2 = '2',
            Num3 = '3',
            Num4 = '4',
            Num5 = '5',
            Num6 = '6',
            Num7 = '7',
            Num8 = '8',
            Num9 = '9',
            A = 'A',
            B = 'B',
            C = 'C',
            D = 'D',
            E = 'E',
            F = 'F',
            G = 'G',
            H = 'H',
            I = 'I',
            J = 'J',
            K = 'K',
            L = 'L',
            M = 'M',
            N = 'N',
            O = 'O',
            P = 'P',
            Q = 'Q',
            R = 'R',
            S = 'S',
            T = 'T',
            U = 'U',
            V = 'V',
            W = 'W',
            X = 'X',
            Y = 'Y',
            Z = 'Z'
        };
    }

    class Event
    {
    public:
        enum Type {
            Empty,

            MouseMoved,
            MousePressed,
            MouseReleased,
            MouseWheel,

            KeyPressed,
            KeyReleased,

            WindowResized,
            WindowFocus,
            WindowClosed
        } type;

        union {
            struct MouseEvent {
                unsigned x, y;
                Mouse::Button button;
                int wheelDelta;
            } mouse;
            Key::Code key;
            struct WindowResize {
                unsigned width, height;
            } wndResize;
            bool focus;
        } data;

    private:
        Event(Type type, unsigned x = 0, unsigned y = 0, Mouse::Button btn = Mouse::ButtonNone, int wheelDelta = 0)
        {
            this->type = type;

            switch (type)
            {
            case MousePressed:
            case MouseReleased:
                data.mouse.button = btn;
                // no break;
            case MouseWheel:
                data.mouse.wheelDelta = wheelDelta;
                // no break;
            case MouseMoved:
                data.mouse.x = x;
                data.mouse.y = y;
                break;
            case WindowResized:
                data.wndResize.width = x;
                data.wndResize.height = y;
                break;
            }
        }

        Event(Key::Code code, bool pressed)
        {
            type = pressed ? KeyPressed : KeyReleased;
            data.key = code;
        }

        Event(bool focus)
        {
            type = WindowFocus;
            this->data.focus = focus;
        }

    public:
        Event(): type(Empty) {}
        static Event mouseMovedEvent(unsigned x, unsigned y) { return Event(MouseMoved, x, y); }
        static Event mousePressedEvent(unsigned x, unsigned y, Mouse::Button btn) { return Event(MousePressed, x, y, btn); }
        static Event mouseReleasedEvent(unsigned x, unsigned y, Mouse::Button btn) { return Event(MouseReleased, x, y, btn); }
        static Event mouseWheelEvent(unsigned x, unsigned y, int delta) { return Event(MouseWheel, x, y, Mouse::ButtonNone, delta); }
        static Event keyPressedEvent(Key::Code code) { return Event(code, true); }
        static Event keyReleasedEvent(Key::Code code) { return Event(code, false); }
        static Event windowResizedEvent(unsigned x, unsigned y) { return Event(WindowResized, x, y); }
        static Event windowFocusEvent(bool focus) { return Event(focus); }
        static Event windowClosedEvent() { return Event(WindowClosed); }
    };
} // namespace sb

#endif //EVENT_H

#ifndef EVENT_H
#define EVENT_H


#include <X11/Xlib.h>

namespace sb
{
    namespace Mouse
    {
        enum Button {
            ButtonNone = 0x0,
            ButtonLeft = Button1,
            ButtonRight = Button3,
            ButtonMiddle = Button2,
            // uh, these two are actually wheel motions
            ButtonX1 = Button4,
            ButtonX2 = Button5
        };
    }

    namespace Key
    {
        enum Code {
            Backspace = XK_BackSpace,
            Esc = XK_Escape,
            Enter = XK_Return,
            CapsLock = XK_Caps_Lock,
            ScrollLock = XK_Scroll_Lock,
            NumLock = XK_Num_Lock,
            Delete = XK_Delete,
            Insert = XK_Insert,
            Pause = XK_Pause,
            PrintScreen = XK_Print,  // TODO: check
            //Sleep = XK_Sleep,      // TODO: find
            Space = XK_space,        // TODO: check
            PageUp = XK_Prior,
            PageDown = XK_Next,
            Home = XK_Home,
            End = XK_End,

            Comma = XK_comma,            /* ,< */
            Period = XK_period,          /* .> */
            Equal = XK_equal,            /* += */
            Minus = XK_minus,            /* -_ */
            Colon = XK_semicolon,        /* ;: */
            Slash = XK_slash,            /* /? */
            Tilde = XK_dead_grave,       /* `~ */
            LBracket = XK_bracketleft,   /* [{ */
            Backslash = XK_backslash,    /* \| */
            RBracket = XK_bracketright,  /* ]} */
            Apostrophe = XK_dead_acute,  /* '" */

            LCtrl = XK_Control_L,
            LAlt = XK_Alt_L,
            LShift = XK_Shift_L,
            LWin = XK_Super_L,    // TODO: check
            RCtrl = XK_Control_R,
            RAlt = XK_Alt_R,
            RShift = XK_Shift_R,
            RWin = XK_Super_R,    // TODO: check
            //Shift = VK_SHIFT,   /* LShift or RShift */

            ArrowDown = XK_Down,
            ArrowRight = XK_Right,
            ArrowLeft = XK_Left,
            ArrowUp = XK_Up,

            F1 = XK_F1,
            F2 = XK_F2,
            F3 = XK_F3,
            F4 = XK_F4,
            F5 = XK_F5,
            F6 = XK_F6,
            F7 = XK_F7,
            F8 = XK_F8,
            F9 = XK_F9,
            F10 = XK_F10,
            F11 = XK_F11,
            F12 = XK_F12,

            Numpad0 = XK_KP_0,
            Numpad1 = XK_KP_1,
            Numpad2 = XK_KP_2,
            Numpad3 = XK_KP_3,
            Numpad4 = XK_KP_4,
            Numpad5 = XK_KP_5,
            Numpad6 = XK_KP_6,
            Numpad7 = XK_KP_7,
            Numpad8 = XK_KP_8,
            Numpad9 = XK_KP_9,
            NumpadDecimal = XK_KP_Separator,
            NumpadMultiply = XK_KP_Multiply,
            NumpadDivide = XK_KP_Divide,
            NumpadSubtract = XK_KP_Subtract,
            NumpadAdd = XK_KP_Add,

            Num0 = XK_0,
            Num1 = XK_1,
            Num2 = XK_2,
            Num3 = XK_3,
            Num4 = XK_4,
            Num5 = XK_5,
            Num6 = XK_6,
            Num7 = XK_7,
            Num8 = XK_8,
            Num9 = XK_9,
            A = XK_a,
            B = XK_b,
            C = XK_c,
            D = XK_d,
            E = XK_e,
            F = XK_f,
            G = XK_g,
            H = XK_h,
            I = XK_i,
            J = XK_j,
            K = XK_k,
            L = XK_l,
            M = XK_m,
            N = XK_n,
            O = XK_o,
            P = XK_p,
            Q = XK_q,
            R = XK_r,
            S = XK_s,
            T = XK_t,
            U = XK_u,
            V = XK_v,
            W = XK_w,
            X = XK_x,
            Y = XK_y,
            Z = XK_z
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
            default:
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

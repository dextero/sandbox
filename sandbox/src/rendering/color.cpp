#include "color.h"

namespace sb
{
    // pre-defined colors
    Color Color::Red = Color(1.f, 0.f, 0.f);
    Color Color::Green = Color(0.f, 1.f, 0.f);
    Color Color::Blue = Color(0.f, 0.f, 1.f);
    Color Color::Yellow = Color(1.f, 1.f, 0.f);
    Color Color::Cyan = Color(0.f, 1.f, 1.f);
    Color Color::Purple = Color(1.f, 0.f, 1.f);
    Color Color::Orange = Color(1.f, 0.5f, 0.f);
    Color Color::White = Color(1.f, 1.f, 1.f);
    Color Color::Black = Color(0.f, 0.f, 0.f);


    Color::Color(): r(0.f), g(0.f), b(0.f), a(1.f)
    {
    }

    Color::Color(float uniform, float a): r(uniform), g(uniform), b(uniform), a(a)
    {
    }

    Color::Color(float r, float g, float b, float a): r(r), g(g), b(b), a(a)
    {
    }

    Color::Color(const Color& c, float a): r(c.r), g(c.g), b(c.b), a(a)
    {
    }

    Color Color::operator *(float factor) const
    {
        return Color(r * factor, g * factor, b * factor, a);
    }

    Color& Color::operator *=(float factor)
    {
        r *= factor;
        g *= factor;
        b *= factor;
        return *this;
    }

    Color Color::operator +(const Color& c) const
    {
        return Color(r + c.r, g + c.g, b + c.b, (a + c.a) / 2.f);
    }

    Color& Color::operator +=(const Color& c)
    {
        r += c.r;
        g += c.g;
        b += c.b;
        a = (a + c.a) / 2.f;
        return *this;
    }
} // namespace sb

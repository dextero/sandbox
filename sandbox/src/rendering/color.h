#ifndef COLOR_H
#define COLOR_H

namespace sb
{
    class Color
    {
    public:
        float r, g, b, a;

        Color();
        explicit Color(float uniform, float a = 1.f);
        Color(float r, float g, float b, float a = 1.f);
        Color(const Color& c, float a);    // change alpha

        Color operator *(float factor) const;
        Color& operator *=(float factor);

        Color operator +(const Color& c) const;
        Color& operator +=(const Color& c);

        static Color Red;
        static Color Green;
        static Color Blue;
        static Color Yellow;
        static Color Cyan;
        static Color Purple;
        static Color Orange;
        static Color White;
        static Color Black;
    };
} // namespace sb

#endif //COLOR_H


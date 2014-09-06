#ifndef UTILS_RECT_H
#define UTILS_RECT_H

#include "utils/vector.h"

namespace sb {

template<typename T>
struct TRect
{
    union {
        TVec2<T> topLeft;
        struct {
            T left;
            T top;
        };
    };
    union {
        TVec2<T> bottomRight;
        struct {
            T right;
            T bottom;
        };
    };

    TRect():
        topLeft(),
        bottomRight()
    {}

    TRect(const TVec2<T>& topLeft,
          const TVec2<T>& size):
        topLeft(topLeft),
        bottomRight(topLeft + size)
    {
        fix();
    }

    TRect(T left, T right, T bottom, T top):
        topLeft(top, left),
        bottomRight(bottom, right)
    {
        fix();
    }

    TRect(const TRect& r): TRect(r.left, r.right, r.bottom, r.top) {}
    TRect& operator =(const TRect& r)
    {
        topLeft = r.topLeft;
        bottomRight = r.bottomRight;
        return *this;
    }

    TRect(TRect&& r): TRect(r.left, r.right, r.bottom, r.top) {}
    TRect& operator =(TRect&& r)
    {
        topLeft = r.topLeft;
        bottomRight = r.bottomRight;
        return *this;
    }

    T width() const
    {
        sbAssert(right > left, "invalid rect");
        return right - left;
    }

    T height() const
    {
        sbAssert(top > bottom, "invalid rect");
        return top - bottom;
    }

    bool contains(T x, T y) const
    {
        sbAssert(right > left, "invalid rect");
        sbAssert(top > bottom, "invalid rect");

        return left <= x && x <= right
                && top >= y && y >= bottom;
    }

    bool contains(const TVec2<T>& v) const
    {
        return contains(v.x, v.y);
    }

    bool isEmpty() const
    {
        return left == right && top == bottom;
    }

    Vec2 topRight() const { return { right, top }; }
    Vec2 bottomLeft() const { return { left, bottom }; }

    TRect& fix()
    {
        if (left > right) {
            std::swap(left, right);
        }
        if (bottom > top) {
            std::swap(top, bottom);
        }
        return *this;
    }
};

typedef TRect<float> FloatRect;
typedef TRect<int> IntRect;

template<typename T>
std::ostream& operator <<(std::ostream& os, const TRect<T>& r)
{
    return os << "[" << r.topLeft << ", " << r.bottomRight << "]";
}

} // namespace sb

#endif // UTILS_RECT_H

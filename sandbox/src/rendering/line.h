#ifndef LINE_H
#define LINE_H

#include "types.h"
#include "drawable.h"

namespace sb
{
    class Line: public Drawable
    {
        const Drawable* mAttachedTo;

    public:
        Line();
        Line(const Vec3& to, const Color& col);
        Line(const Vec3& from, const Vec3& to, const Color& col);

        void Set(const Vec3& from, const Vec3& to);
        void AttachTo(const Drawable* drawable);
        void Update();    // synchronize position with attached drawable
    };
} // namespace sb

#endif // LINE_H

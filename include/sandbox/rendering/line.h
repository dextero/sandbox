#ifndef LINE_H
#define LINE_H

#include <memory>

#include "types.h"
#include "drawable.h"

namespace sb
{
    class Line: public Drawable
    {
    public:
        Line(const std::shared_ptr<Shader>& shader);
        Line(const Vec3& to,
             const Color& col,
             const std::shared_ptr<Shader>& shader);
        Line(const Vec3& from,
             const Vec3& to,
             const Color& col,
             const std::shared_ptr<Shader>& shader);

        Line(const Line&) = default;
        Line(Line&&) = default;
        Line& operator =(const Line&) = default;
        Line& operator =(Line&&) = default;

        virtual ~Line() {}

        void set(const Vec3& from,
                 const Vec3& to);
        void attachTo(const std::shared_ptr<Drawable> &drawable);
        void update();    // synchronize position with attached drawable

    private:
        std::shared_ptr<const Drawable> mAttachedTo;
    };
} // namespace sb

#endif // LINE_H

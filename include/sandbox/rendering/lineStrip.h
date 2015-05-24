#ifndef LINESTRIP_H
#define LINESTRIP_H

#include <memory>

#include "types.h"
#include "drawable.h"

namespace sb
{
    class LineStrip: public Drawable
    {
    public:
        LineStrip(const std::vector<Vec3>& vertices,
                  const Color& col,
                  const std::shared_ptr<Shader>& shader);

        LineStrip(const LineStrip&) = default;
        LineStrip(LineStrip&&) = default;
        LineStrip& operator =(const LineStrip&) = default;
        LineStrip& operator =(LineStrip&&) = default;

        virtual ~LineStrip() {}
    };
} // namespace sb

#endif // LINESTRIP_H

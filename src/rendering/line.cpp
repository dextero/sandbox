#include <sandbox/rendering/line.h>

namespace sb
{
    Line::Line(const std::shared_ptr<Shader>& shader):
        Line(Vec3(), Vec3(), Color(), shader)
    {}

    Line::Line(const Vec3& to,
               const Color& col,
               const std::shared_ptr<Shader>& shader):
        Line(Vec3(), to, col, shader)
    {}

    Line::Line(const Vec3& from,
               const Vec3& to,
               const Color& col,
               const std::shared_ptr<Shader>& shader):
        Drawable(ProjectionType::Perspective,
                 gResourceMgr.getLine(),
                 nullptr,
                 shader),
        mAttachedTo(NULL)
    {
        mMesh = gResourceMgr.getLine();
        mColor = col;

        setScale(to - from);
        setPosition(from);
    }

    void Line::set(const Vec3& from,
                   const Vec3& to)
    {
        setScale(to - from);
        setPosition(from);
        setRotation(0.0_rad, 0.0_rad, 0.0_rad);
    }

    void Line::attachTo(const std::shared_ptr<Drawable> &drawable)
    {
        mAttachedTo = drawable;
    }

    // synchronize position with attached drawable
    void Line::update()
    {
        if (!mAttachedTo)
            return;

        setPosition(mAttachedTo->getPosition());
    }
} // namespace sb

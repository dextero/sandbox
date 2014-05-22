#include "line.h"

namespace sb
{
    Line::Line():
        Drawable(ProjectionPerspective),
        mAttachedTo(NULL)
    {
        mMesh = gResourceMgr.getLine();
    }

    Line::Line(const Vec3& to, const Color& col):
        Drawable(ProjectionPerspective),
        mAttachedTo(NULL)
    {
        mMesh = gResourceMgr.getLine();
        mColor = col;

        setScale(to);
    }

    Line::Line(const Vec3& from, const Vec3& to, const Color& col):
        Drawable(ProjectionPerspective),
        mAttachedTo(NULL)
    {
        mMesh = gResourceMgr.getLine();
        mColor = col;

        setScale(to - from);
        setPosition(from);
    }

    void Line::set(const Vec3& from, const Vec3& to)
    {
        setScale(to - from);
        setPosition(from);
        setRotation(0.0_rad, 0.0_rad, 0.0_rad);
    }

    void Line::attachTo(const Drawable* drawable)
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

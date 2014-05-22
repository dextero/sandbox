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

        SetScale(to);
    }

    Line::Line(const Vec3& from, const Vec3& to, const Color& col):
        Drawable(ProjectionPerspective),
        mAttachedTo(NULL)
    {
        mMesh = gResourceMgr.getLine();
        mColor = col;

        SetScale(to - from);
        SetPosition(from);
    }

    void Line::Set(const Vec3& from, const Vec3& to)
    {
        SetScale(to - from);
        SetPosition(from);
        SetRotation(0.0_rad, 0.0_rad, 0.0_rad);
    }

    void Line::AttachTo(const Drawable* drawable)
    {
        mAttachedTo = drawable;
    }

    // synchronize position with attached drawable
    void Line::Update()
    {
        if (!mAttachedTo)
            return;

        SetPosition(mAttachedTo->GetPosition());
    }
} // namespace sb

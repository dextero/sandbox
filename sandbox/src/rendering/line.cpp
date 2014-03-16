#include "line.h"
#include "../utils/profiler.h"

namespace sb
{
    Line::Line():
        Drawable(ProjectionPerspective),
        mAttachedTo(NULL)
    {
        PROFILE();

        mMesh = gResourceMgr.GetLine();
    }

    Line::Line(const Vec3& to, const Color& col):
        Drawable(ProjectionPerspective),
        mAttachedTo(NULL)
    {
        PROFILE();

        mMesh = gResourceMgr.GetLine();
        mColor = col;

        SetScale(to);
    }

    Line::Line(const Vec3& from, const Vec3& to, const Color& col):
        Drawable(ProjectionPerspective),
        mAttachedTo(NULL)
    {
        PROFILE();

        mMesh = gResourceMgr.GetLine();
        mColor = col;

        SetScale(to - from);
        SetPosition(from);
    }

    void Line::Set(const Vec3& from, const Vec3& to)
    {
        PROFILE();

        SetScale(to - from);
        SetPosition(from);
        SetRotation(0.f, 0.f, 0.f);
    }

    void Line::AttachTo(const Drawable* drawable)
    {
        PROFILE();

        mAttachedTo = drawable;
    }

    // synchronize position with attached drawable
    void Line::Update()
    {
        PROFILE();

        if (!mAttachedTo)
            return;

        SetPosition(mAttachedTo->GetPosition());
    }
} // namespace sb

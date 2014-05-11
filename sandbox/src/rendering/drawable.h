#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "types.h"
#include "resources/resourceMgr.h"
#include "color.h"
#include "shader.h"

#include <vector>


namespace sb
{
    class Drawable
    {
    protected:
        Mesh* mMesh;
        TextureId mTexture;    // if present, overrides model's own texture
        Color mColor;

        enum EDrawableFlags {
            FlagPositionChanged = 1,
            FlagScaleChanged = 1 << 1,
            FlagRotationChanged = 1 << 2,
            FlagTransformationChanged = 0x7
        };

        Mat44 mTranslationMatrix, mScaleMatrix, mRotationMatrix;
        Mat44 mTransformationMatrix;
        int mFlags;

        Vec3 mPosition, mScale;
        Quat mRotation;

    protected:
        enum EProjectionType {
            ProjectionOrthographic,
            ProjectionPerspective
        } mProjectionType;

        Drawable(EProjectionType projType);

        void RecalculateMatrices();

    public:
        Drawable();
        ~Drawable();
        Drawable(const Drawable& copy);

        const Vec3& GetPosition() const;
        const Vec3 GetRotationAxis() const;
        Radians GetRotationAngle() const;
        void GetRotationAxisAngle(Vec3& axis, Radians& angle) const;
        const Quat& GetRotationQuaternion() const;
        const Vec3& GetScale() const;
        Shader::EShader GetShader() const;

        void SetPosition(const Vec3& pos);
        void SetPosition(float x, float y, float z);
        void SetRotation(const Vec3& axis, Radians angle);
        void SetRotation(Radians x, Radians y, Radians z);
        void SetScale(const Vec3& scale);
        void SetScale(float x, float y, float z);
        void SetScale(float uniform);
        void Rotate(Radians angle);
        void Rotate(const Vec3& axis, Radians angle);
        const Mat44& GetTransformationMatrix();

    friend class Renderer;
    };
} // namespace sb

#endif //DRAWABLE_H

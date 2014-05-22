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
        std::shared_ptr<Mesh> mMesh;
        std::shared_ptr<TextureId> mTexture;    // if present, overrides model's own texture
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

        void recalculateMatrices();

    public:
        Drawable();

        const Vec3& getPosition() const;
        const Vec3 getRotationAxis() const;
        Radians getRotationAngle() const;
        void getRotationAxisAngle(Vec3& axis, Radians& angle) const;
        const Quat& getRotationQuaternion() const;
        const Vec3& getScale() const;
        Shader::EShader getShader() const;

        void setPosition(const Vec3& pos);
        void setPosition(float x, float y, float z);
        void setRotation(const Vec3& axis, Radians angle);
        void setRotation(Radians x, Radians y, Radians z);
        void setScale(const Vec3& scale);
        void setScale(float x, float y, float z);
        void setScale(float uniform);
        void rotate(Radians angle);
        void rotate(const Vec3& axis, Radians angle);
        const Mat44& getTransformationMatrix();

    friend class Renderer;
    };
} // namespace sb

#endif //DRAWABLE_H

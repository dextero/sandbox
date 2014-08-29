#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "rendering/types.h"
#include "rendering/color.h"
#include "rendering/shader.h"
#include "rendering/renderer.h"
#include "resources/resourceMgr.h"

#include <vector>


namespace sb
{
    class Drawable
    {
    public:
        Drawable() = delete;
        virtual ~Drawable() {}

        Drawable(const Drawable&) = default;
        Drawable(Drawable&&) = default;
        Drawable& operator =(const Drawable&) = default;
        Drawable& operator =(Drawable&&) = default;

        const Vec3& getPosition() const;
        const Vec3 getRotationAxis() const;
        Radians getRotationAngle() const;
        void getRotationAxisAngle(Vec3& axis, Radians& angle) const;
        const Quat& getRotationQuaternion() const;
        const Vec3& getScale() const;

        void setPosition(const Vec3& pos);
        void setPosition(float x, float y, float z);
        void setRotation(const Vec3& axis, Radians angle);
        void setRotation(Radians x, Radians y, Radians z);
        void setScale(const Vec3& scale);
        void setScale(float x, float y, float z);
        void setScale(float uniform);
        void rotate(Radians angle);
        void rotate(const Vec3& axis, Radians angle);
        const Mat44& getTransformationMatrix() const;

        bool operator <(const Drawable& d) const;

    protected:
        std::shared_ptr<Mesh> mMesh;
        std::shared_ptr<Texture> mTexture;    // if present, overrides model's own texture
        std::shared_ptr<Shader> mShader;
        Color mColor;

        enum EDrawableFlags {
            FlagPositionChanged = 1,
            FlagScaleChanged = 1 << 1,
            FlagRotationChanged = 1 << 2,
            FlagTransformationChanged = 0x7
        };

        mutable Mat44 mTranslationMatrix;
        mutable Mat44 mScaleMatrix;
        mutable Mat44 mRotationMatrix;
        mutable Mat44 mTransformationMatrix;
        mutable int mFlags;

        Vec3 mPosition;
        Vec3 mScale;
        Quat mRotation;

        ProjectionType mProjectionType;

        Drawable(ProjectionType projType,
                 const std::shared_ptr<Mesh>& mesh,
                 const std::shared_ptr<Texture>& texture,
                 const std::shared_ptr<Shader>& shader);

        void recalculateMatrices() const;

        void draw(Renderer::State& rendererState) const;

        friend class Renderer;
    };
} // namespace sb

#endif //DRAWABLE_H

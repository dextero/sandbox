#ifndef SIM_BALL_H
#define SIM_BALL_H

#include <sandbox/utils/types.h>
#include <sandbox/rendering/model.h>
#include <sandbox/rendering/line.h>
#include <sandbox/rendering/renderer.h>

#include <list>

namespace Sim
{
    extern const sb::Color& ColorVelocity;
    extern const sb::Color& ColorVelocityRelative;
    extern const sb::Color& ColorGravity;
    extern const sb::Color& ColorDrag;
    extern const sb::Color& ColorWind;
    extern const sb::Color& ColorBuoyancy;
    extern const sb::Color& ColorNet;
    extern const sb::Color& ColorThrow;
    extern const sb::Color& ColorPath;

    class Ball
    {
    public:
        typedef std::pair<sb::Vec3d, sb::Line> ColVec;

        ColVec mVelocity;
        ColVec mAccGravity;
        ColVec mAccDrag;
        ColVec mAccWind;
        ColVec mAccBuoyancy;
        ColVec mAccNet;

        double mMass;
        double mRadius;
        double mArea;
        double mVolume;
        double mDistanceCovered;
        double mHorizontalDistanceCovered;
        double mDeltaDistance; // used for drawing ball path
        double mTime;
        double mTotalEnergy;

        sb::Vec3d mPos;
        std::shared_ptr<sb::Model> mModel;

        uint32_t mTimeToLive; // after how many ball-ground collisions should we erase the ball?

        Ball(const sb::Vec3d& pos,
             const sb::Vec3d& velocity,
             double mass,
             double radius,
             const std::shared_ptr<sb::Shader>& modelShader,
             const std::shared_ptr<sb::Shader>& lineShader);

        void set(ColVec& what,
                 const sb::Vec3d& value,
                 bool scaleToForce = false);
        bool update(double dt,
                    const sb::Vec3d& gravity,
                    const sb::Vec3d& wind,
                    double fluidDensity = 1.204, // 1.204 - mass density of air at 1 atm, 20*C
                    double maxPathLength = 10.0,
                    bool forces = false); // forces - should it draw forces or accelerations?
        void drawAll(sb::Renderer& r);

    private:
        std::list<sb::Vec3d> mPath;
        std::shared_ptr<sb::Shader> mLineShader;

        void attachLines();
    };
}

#endif // SIM_BALL_H

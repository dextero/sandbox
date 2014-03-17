#ifndef SIM_BALL_H
#define SIM_BALL_H

#include "utils/types.h"
#include "../rendering/model.h"
#include "../rendering/line.h"
#include "../rendering/renderer.h"

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
        std::list<Vec3d> mPath;

        void AttachLines();

    public:
        typedef std::pair<Vec3d, sb::Line> ColVec;

        ColVec mVelocity,
               mAccGravity,
               mAccDrag,
               mAccWind,
               mAccBuoyancy,
               mAccNet;

        double mMass,
               mRadius,
               mArea,
               mVolume,
               mDistanceCovered,
               mHorizontalDistanceCovered,
               mDeltaDistance,        // used for drawing ball path
               mTime,
               mTotalEnergy;

        Vec3d mPos;
        sb::Model mModel;

        uint32_t mTimeToLive;    // after how many ball-ground collisions should we erase the ball?

        Ball(const Vec3d& pos, const Vec3d& velocity, double mass, double radius);
        Ball(const Ball& copy);

        void Set(ColVec& what, const Vec3d& value, bool scaleToForce = false);
        // 1.204 - mass density of air at 1 atm, 20*C
        bool Update(double dt, const Vec3d& gravity, const Vec3d& wind, double fluidDensity = 1.204, double maxPathLength = 10.0, bool forces = false); // forces - should it draw forces or accelerations?
        void DrawAll(sb::Renderer& r);
    };
}

#endif // SIM_BALL_H

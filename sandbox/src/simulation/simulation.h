#ifndef SIM_SIMULATION_H
#define SIM_SIMULATION_H

#include <list>
#include "../utils/types.h"
#include "../rendering/line.h"

namespace sb
{
    class Renderer;
}

namespace Sim
{
    class Ball;

    class Simulation
    {
    public:
        enum ESimType {
            SimSingleThrow,
            SimContiniousThrow
        };

        enum EVectorDisplayType {
            DisplayForce,
            DisplayAcceleration
        };

    private:
        std::list<Ball*> mBalls;
        std::list<std::list<Ball*>::iterator> mMarkedForDelete;

        Vec3d mThrowStartPos, mThrowStartVelocity;
        Vec3d mGravity, mWindVelocity;
        sb::Line mThrowStartLine, mGravityLine, mWindVelocityLine;

    public:
        double mAirDensity;
        ESimType mSimType;
        float mBallThrowDelay, mBallThrowAccumulator;
        uint mMaxBalls;
        float mSloMoFactor;

        double mBallMass, mBallRadius, mBallPathLength;

        bool mPaused, mShowLauncherLines, mPauseOnGroundHit;
        EVectorDisplayType mVectorDisplayType;

        Simulation(ESimType type);
        ~Simulation();

        void Update(float dt);
        void SetThrowStart(const Vec3d& pos, const Vec3d& v);
        void SetGravity(const Vec3d& g);
        void SetWind(const Vec3d& w);
        void DrawAll(sb::Renderer& renderer);
        void TogglePause();
        void Reset();

        // returns lines displayed
        uint PrintParametersToScreen(float x = 0.f, float y = 0.f, uint line = 0u);
        const Ball* Raycast(const Vec3& rayOrig, const Vec3& rayDir);
        uint PrintBallParametersToScreen(const Ball* ball, float x = 0.f, float y = 0.f, uint line = 0u);
    };
}

#endif //SIM_SIMULATION_H

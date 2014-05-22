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
        uint32_t mMaxBalls;
        float mSloMoFactor;

        double mBallMass, mBallRadius, mBallPathLength;

        bool mPaused, mShowLauncherLines, mPauseOnGroundHit;
        EVectorDisplayType mVectorDisplayType;

        Simulation(ESimType type);
        ~Simulation();

        void update(float dt);
        void setThrowStart(const Vec3d& pos, const Vec3d& v);
        void setGravity(const Vec3d& g);
        void setWind(const Vec3d& w);
        void drawAll(sb::Renderer& renderer);
        void togglePause();
        void reset();

        // returns lines displayed
        uint32_t printParametersToScreen(float x = 0.f, float y = 0.f, uint32_t line = 0u);
        const Ball* raycast(const Vec3& rayOrig, const Vec3& rayDir);
        uint32_t printBallParametersToScreen(const Ball* ball, float x = 0.f, float y = 0.f, uint32_t line = 0u);
    };
}

#endif //SIM_SIMULATION_H

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

        Simulation(ESimType type,
                   const std::shared_ptr<sb::Shader>& ballShader,
                   const std::shared_ptr<sb::Shader>& lineShader);
        ~Simulation();

        void update(float dt);
        void setThrowStart(const Vec3d& pos, const Vec3d& v);
        void setGravity(const Vec3d& g);
        void setWind(const Vec3d& w);
        void drawAll(sb::Renderer& renderer);
        void togglePause();
        void reset();

        // returns lines displayed
        uint32_t printParametersToScreen(float x = 0.f,
                                         float y = 0.f,
                                         uint32_t line = 0u);
        const Ball* raycast(const Vec3& rayOrig,
                            const Vec3& rayDir);
        uint32_t printBallParametersToScreen(const Ball* ball,
                                             float x = 0.f,
                                             float y = 0.f,
                                             uint32_t line = 0u);

        double getBallRadius() const { return mBallRadius; }

        void increaseBallPathLength(double delta)
        {
            mBallPathLength = glm::clamp(mBallPathLength + delta, 0.0, 1000.0);
        }
        void increaseMaxBalls(ssize_t delta)
        {
            mMaxBalls = glm::clamp((ssize_t)mMaxBalls + delta, (ssize_t)1, (ssize_t)1000);
        }
        void increaseAirDensity(double deltaSeconds) { mAirDensity += deltaSeconds; }
        void increaseBallThrowDelay(double deltaSeconds) { mBallThrowDelay += deltaSeconds; }
        void increaseSloMoFactor(double delta) { mSloMoFactor += delta; }
        void increaseBallRadius(double delta) { mBallRadius += delta; }
        void increaseBallMass(double delta) { mBallMass += delta; }

        void toggleVectorDisplayType()
        {
            if (mVectorDisplayType == DisplayForce) {
                mVectorDisplayType = DisplayAcceleration;
            } else {
                mVectorDisplayType = DisplayForce;
            }
        }

        void togglePaused() { mPaused = !mPaused; }
        void toggleShowLauncherLines() { mShowLauncherLines = !mShowLauncherLines; }
        void togglePauseOnGroundHit() { mPauseOnGroundHit = !mPauseOnGroundHit; }

    private:
        std::shared_ptr<sb::Shader> mBallShader;
        std::shared_ptr<sb::Shader> mLineShader;

        std::list<Ball*> mBalls;
        std::list<std::list<Ball*>::iterator> mMarkedForDelete;

        Vec3d mThrowStartPos;
        Vec3d mThrowStartVelocity;
        Vec3d mGravity;
        Vec3d mWindVelocity;
        sb::Line mThrowStartLine;
        sb::Line mGravityLine;
        sb::Line mWindVelocityLine;

        double mAirDensity;
        ESimType mSimType;
        float mBallThrowDelay;
        float mBallThrowAccumulator;
        uint32_t mMaxBalls;
        float mSloMoFactor;

        double mBallMass;
        double mBallRadius;
        double mBallPathLength;

        bool mPaused;
        bool mShowLauncherLines;
        bool mPauseOnGroundHit;
        EVectorDisplayType mVectorDisplayType;
    };
}

#endif //SIM_SIMULATION_H

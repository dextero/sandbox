#ifndef SIM_SIMULATION_H
#define SIM_SIMULATION_H

#include <list>
#include <sandbox/utils/types.h>
#include <sandbox/rendering/line.h>
#include <sandbox/window/window.h>

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

        void update(float dt);
        void setThrowStart(const sb::Vec3d& pos, const sb::Vec3d& v);
        void setGravity(const sb::Vec3d& g);
        void setWind(const sb::Vec3d& w);
        void drawAll(sb::Renderer& renderer);
        void togglePause();
        void reset();

        // returns lines displayed
        uint32_t printParametersToScreen(sb::Window& wnd,
                                         const sb::Vec2& topLeft = sb::Vec2(0.0f, 0.0f),
                                         uint32_t line = 0u);
        const std::shared_ptr<Ball> raycast(const sb::Vec3& rayOrig,
                                            const sb::Vec3& rayDir);
        uint32_t printBallParametersToScreen(sb::Window& wnd,
                                             const std::shared_ptr<Ball> &ball,
                                             const sb::Vec2& topLeft = sb::Vec2(0.0f, 0.0f),
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

        std::list<std::shared_ptr<Ball>> mBalls;

        sb::Vec3d mThrowStartPos;
        sb::Vec3d mThrowStartVelocity;
        sb::Vec3d mGravity;
        sb::Vec3d mWindVelocity;
        std::shared_ptr<sb::Line> mThrowStartLine;
        std::shared_ptr<sb::Line> mGravityLine;
        std::shared_ptr<sb::Line> mWindVelocityLine;

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

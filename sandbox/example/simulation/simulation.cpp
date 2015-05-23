#include "simulation.h"

#include "ball.h"
#include "rendering/string.h"
#include "utils/stringUtils.h"
#include "utils/logger.h"

using sb::utils::toString;

namespace Sim
{
    Simulation::Simulation(ESimType type,
                           const std::shared_ptr<sb::Shader>& ballShader,
                           const std::shared_ptr<sb::Shader>& lineShader):
        mBallShader(ballShader),
        mLineShader(lineShader),
        mBalls(),
        mThrowStartPos(Vec3d(0., 0., 0.)),
        mThrowStartVelocity(Vec3d(0., 0., 0.)),
        mGravity(),
        mWindVelocity(),
        mThrowStartLine(std::make_shared<sb::Line>(Vec3(1.f, 1.f, 1.f),
                                                   sb::Color(ColorThrow, 0.4f),
                                                   lineShader)),
        mGravityLine(std::make_shared<sb::Line>(Vec3(1.f, 1.f, 1.f),
                                                sb::Color(ColorGravity, 0.4f),
                                                lineShader)),
        mWindVelocityLine(std::make_shared<sb::Line>(Vec3(1.f, 1.f, 1.f),
                                                     sb::Color(ColorWind, 0.4f),
                                                     lineShader)),
        mAirDensity(1.204),
        mSimType(type),
        mBallThrowDelay(0.f),
        mBallThrowAccumulator(0.f),
        mMaxBalls(25u),
        mSloMoFactor(1.f),
        mBallMass(1.),
        mBallRadius(0.5),
        mBallPathLength(10.0),
        mPaused(false),
        mShowLauncherLines(true),
        mPauseOnGroundHit(false),
        mVectorDisplayType(DisplayForce)
    {
        gLog.info("starting simulation, type: %d\n", (int)type);

        switch (mSimType)
        {
        case SimSingleThrow:
            mBallThrowDelay = -1.f;
            mBallPathLength = 500.0;
            mMaxBalls = 1u;
            mPauseOnGroundHit = true;
            break;
        case SimContiniousThrow:
            mBallThrowDelay = 3.f;
            mBallPathLength = 0.0;
            mMaxBalls = 25u;
            mPauseOnGroundHit = false;
            break;
        default:
            break;
        }

        setThrowStart(Vec3d(0., 1., 0.), Vec3d(20., 20., 0.));
        setGravity(Vec3d(0., -9.81, 0.));
        setWind(Vec3d(0., 0., 0.));
    }

    void Simulation::update(float dt)
    {
        if (mPaused)
            return;

        if (mSloMoFactor > 0.f && (mSimType == SimContiniousThrow || mBalls.empty()))
        {
            mBallThrowAccumulator += dt;
            if (mBallThrowAccumulator >= mBallThrowDelay && mBalls.size() < mMaxBalls)
            {
                mBalls.push_back(std::make_shared<Ball>(mThrowStartPos,
                                                        mThrowStartVelocity,
                                                        mBallMass,
                                                        mBallRadius,
                                                        mBallShader,
                                                        mLineShader));
                mBallThrowAccumulator = 0.f;
            }
        }

        dt *= mSloMoFactor;

        for (auto it = mBalls.begin(); it != mBalls.end();) {
            // returns true on collision with ground
            if ((*it)->update(dt, mGravity, mWindVelocity, mAirDensity,
                              mBallPathLength, mVectorDisplayType == DisplayForce)) {
                it = mBalls.erase(it);
                if (mPauseOnGroundHit)
                    mPaused = true;
            } else {
                ++it;
            }
        }
    }

    void Simulation::setThrowStart(const Vec3d& pos, const Vec3d& v)
    {
        gLog.info("simulation: throw_start set to %s\n", toString(v).c_str());

        mThrowStartLine->setPosition(pos);
        mThrowStartLine->setScale(v);
        mThrowStartPos = pos;
        mThrowStartVelocity = v;

        mGravityLine->attachTo(mThrowStartLine);
        mWindVelocityLine->attachTo(mThrowStartLine);
        mGravityLine->update();
        mWindVelocityLine->update();
    }

    void Simulation::setGravity(const Vec3d& g)
    {
        gLog.info("simulation: gravity set to %s\n", toString(g).c_str());

        mGravityLine->setScale(g);
        mGravity = g;
    }

    void Simulation::setWind(const Vec3d& w)
    {
        gLog.info("simulation: wind set to %s\n", toString(w).c_str());

        mWindVelocityLine->setScale(w);
        mWindVelocity = w;
    }

    void Simulation::drawAll(sb::Renderer& renderer)
    {
        if (mShowLauncherLines) {
            renderer.draw(*mThrowStartLine);
            renderer.draw(*mGravityLine);
            renderer.draw(*mWindVelocityLine);
        }

        for (auto &ball_ptr: mBalls) {
            ball_ptr->drawAll(renderer);
        }
    }

    void Simulation::togglePause()
    {
        mPaused = !mPaused;
    }

    void Simulation::reset()
    {
        mBalls.clear();
    }

    // returns lines displayed
    uint32_t Simulation::printParametersToScreen(sb::Window& wnd,
                                                 const Vec2& topLeft,
                                                 uint32_t line)
    {
        wnd.drawString(sb::utils::format("velocity = {0} ({1})",
                                         mThrowStartVelocity,
                                         mThrowStartVelocity.length()),
                       topLeft, ColorVelocity, line++);
        wnd.drawString(sb::utils::format("gravity = {0} ({1})",
                                         mGravity,
                                         mGravity.length()),
                       topLeft, ColorGravity, line++);
        wnd.drawString("drag", topLeft, ColorDrag, line++);
        wnd.drawString(sb::utils::format("wind velocity = {0} ({1})",
                                         mWindVelocity,
                                         mWindVelocity.length()),
                       topLeft, ColorWind, line++);
        wnd.drawString("buoyancy", topLeft, ColorBuoyancy, line++);
        wnd.drawString("net", topLeft, ColorNet, line++);
        wnd.drawString("trajectory", topLeft, ColorPath, line++);
        // additional line
        wnd.drawString(sb::utils::format(
                           "simulation type = {0}\n"
                           "throw delay = {1}\n"
                           "throw accumulator = {2}\n"
                           "max balls = {3}\n"
                           "slomo factor = {4}\n"
                           "air density = {5}\n"
                           "throw start pos = {6}\n"
                           "ball throw velocity = {7}\n"
                           "ball mass = {8}\n"
                           "ball radius = {9}\n"
                           "ball path length = {10}\n"
                           "paused = {11}\n"
                           "show throw lines = {12}\n"
                           "vector display type = {13}\n"
                           "auto-pause on ground hit = {14}",
                           mSimType == SimSingleThrow ? "single ball"
                                                      : "multiple balls",
                           mBallThrowDelay, mBallThrowAccumulator,
                           mMaxBalls, mSloMoFactor, mAirDensity,
                           mThrowStartPos, mThrowStartVelocity, mBallMass,
                           mBallRadius, mBallPathLength, mPaused,
                           mShowLauncherLines,
                           mVectorDisplayType == DisplayForce ? "forces"
                                                              : "accelerations",
                           mPauseOnGroundHit),
                       topLeft, sb::Color::White, line);
        line += 15;
        return line;
    }

    const std::shared_ptr<Ball> Simulation::raycast(const Vec3& rayOrig,
                                                    const Vec3& rayDir)
    {
        Vec3d orig = Vec3d(rayOrig);
        Vec3d dir = Vec3d(rayDir);
        dir = dir.normalized();

        double intersection = std::numeric_limits<double>::infinity();
        std::shared_ptr<Ball> ret;
        for (auto &ball_ptr: mBalls)
        {
            Vec3d origToCenter(orig - ball_ptr->mPos);
            double b = -dir.dot(origToCenter);
            double det = b * b - origToCenter.dot(origToCenter)
                         + ball_ptr->mRadius * ball_ptr->mRadius;

            if (det < 0.)
                continue;
            det = sqrt(det);

            double t1 = b + det;
            double t2 = b - det;

            if (t1 < 0.)
                continue;

            double minVal = (t2 > 0. ? t2 : t1);

            if (minVal < intersection)
            {
                intersection = minVal;
                ret = ball_ptr;
            }
        }

        return ret;
    }

    uint32_t Simulation::printBallParametersToScreen(
            sb::Window& wnd,
            const std::shared_ptr<Ball> &ball,
            const Vec2& topLeft,
            uint32_t line)
    {
        if (ball)
        {
            wnd.drawString(sb::utils::format("velocity = {0}", ball->mVelocity.first),
                           topLeft, ColorVelocity, line++);
            wnd.drawString(sb::utils::format("gravity = {0}", ball->mAccGravity.first),
                           topLeft, ColorGravity, line++);
            wnd.drawString(sb::utils::format("drag = {0}", ball->mAccDrag.first),
                           topLeft, ColorDrag, line++);
            wnd.drawString(sb::utils::format("wind = {0}", ball->mAccWind.first),
                           topLeft, ColorWind, line++);
            wnd.drawString(sb::utils::format("buoyancy = {0}", ball->mAccBuoyancy.first),
                           topLeft, ColorBuoyancy, line++);
            wnd.drawString(sb::utils::format("net = {0}", ball->mAccNet.first),
                           topLeft, ColorNet, line++);
            wnd.drawString(sb::utils::format("distance covered = {0}", ball->mDistanceCovered),
                           topLeft, ColorPath, line++);
            wnd.drawString(sb::utils::format(
                               "mass = {0}\n"
                               "radius = {1}\n"
                               "area = {2}\n"
                               "volume = {3}\n"
                               "horizontal distance covered = {4}\n"
                               "energy = {5}\n"
                               "time = {6}\n"
                               "TTL = {7}",
                               ball->mMass, ball->mRadius, ball->mArea,
                               ball->mVolume, ball->mHorizontalDistanceCovered,
                               ball->mTotalEnergy, ball->mTime, ball->mTimeToLive),
                           topLeft, sb::Color::White, line);
            line += 9;
        } else {
           wnd.drawString("no ball selected", topLeft, sb::Color::White, line++);
        }

        return line;
    }

}

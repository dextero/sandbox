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
        mThrowStartPos(Vec3d(0., 0., 0.)),
        mThrowStartVelocity(Vec3d(0., 0., 0.)),
        mThrowStartLine(Vec3(1.f, 1.f, 1.f),
                        sb::Color(ColorThrow, 0.4f),
                        lineShader),
        mGravityLine(Vec3(1.f, 1.f, 1.f),
                     sb::Color(ColorGravity, 0.4f),
                     lineShader),
        mWindVelocityLine(Vec3(1.f, 1.f, 1.f),
                          sb::Color(ColorWind, 0.4f),
                          lineShader),
        mAirDensity(1.204),
        mSimType(type),
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

    Simulation::~Simulation()
    {
        for (std::list<Ball*>::iterator it = mBalls.begin(); it != mBalls.end(); ++it)
            delete (*it);
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
                mBalls.push_back(new Ball(mThrowStartPos,
                                          mThrowStartVelocity,
                                          mBallMass,
                                          mBallRadius,
                                          mBallShader,
                                          mLineShader));
                mBallThrowAccumulator = 0.f;
            }
        }

        dt *= mSloMoFactor;

        for (std::list<Ball*>::iterator it = mBalls.begin(); it != mBalls.end(); ++it)
            if ((*it)->update(dt, mGravity, mWindVelocity, mAirDensity, mBallPathLength, mVectorDisplayType == DisplayForce)) // returns true on collision with ground
            {
                mMarkedForDelete.push_back(it);
                if (mPauseOnGroundHit)
                    mPaused = true;
            }

        if (!mPaused)
        {
            for(std::list<std::list<Ball*>::iterator>::iterator it = mMarkedForDelete.begin(); it != mMarkedForDelete.end(); ++it)
            {
                SAFE_RELEASE(*(*it));
                mBalls.erase(*it);
            }
            mMarkedForDelete.clear();
        }
    }

    void Simulation::setThrowStart(const Vec3d& pos, const Vec3d& v)
    {
        gLog.info("simulation: throw_start set to %s\n", toString(v).c_str());

        mThrowStartLine.setPosition((float)pos[0], (float)pos[1], (float)pos[2]);
        mThrowStartLine.setScale((float)v[0], (float)v[1], (float)v[2]);
        mThrowStartPos = pos;
        mThrowStartVelocity = v;

        mGravityLine.attachTo(&mThrowStartLine);
        mWindVelocityLine.attachTo(&mThrowStartLine);
        mGravityLine.update();
        mWindVelocityLine.update();
    }

    void Simulation::setGravity(const Vec3d& g)
    {
        gLog.info("simulation: gravity set to %s\n", toString(g).c_str());

        mGravityLine.setScale((float)g[0], (float)g[1], (float)g[2]);
        mGravity = g;
    }

    void Simulation::setWind(const Vec3d& w)
    {
        gLog.info("simulation: wind set to %s\n", toString(w).c_str());

        mWindVelocityLine.setScale((float)w[0], (float)w[1], (float)w[2]);
        mWindVelocity = w;
    }

    void Simulation::drawAll(sb::Renderer& renderer)
    {
        if (mShowLauncherLines)
        {
            renderer.draw(mThrowStartLine);
            renderer.draw(mGravityLine);
            renderer.draw(mWindVelocityLine);
        }

        for (std::list<Ball*>::iterator it = mBalls.begin(); it != mBalls.end(); ++it)
            (*it)->drawAll(renderer);
    }

    void Simulation::togglePause()
    {
        mPaused = !mPaused;
    }

    void Simulation::reset()
    {
        for (std::list<Ball*>::iterator it = mBalls.begin(); it != mBalls.end(); ++it)
            delete (*it);

        mBalls.clear();
    }

    // returns lines displayed
    uint32_t Simulation::printParametersToScreen(float x, float y, uint32_t line)
    {
        using sb::utils::makeString;

        sb::String::print(makeString("velocity = ", mThrowStartVelocity,
                                     " (", mThrowStartVelocity.length(), ")"),
                          x, y, ColorVelocity, line++);
        sb::String::print(makeString("gravity = ", mGravity,
                                     " (", mGravity.length(), ")"),
                          x, y, ColorGravity, line++);
        sb::String::print("drag", x, y, ColorDrag, line++);
        sb::String::print(makeString("wind velocity = ", mWindVelocity,
                                     " (", mWindVelocity.length(), ")"),
                          x, y, ColorWind, line++);
        sb::String::print("buoyancy", x, y, ColorBuoyancy, line++);
        sb::String::print("net", x, y, ColorNet, line++);
        sb::String::print("trajectory", x, y, ColorPath, line++);
        // additional line
        sb::String::print(
                makeString("simulation type = ",
                           mSimType == SimSingleThrow ? "single ball"
                                                      : "multiple balls",
                           "\nthrow delay = ", mBallThrowDelay,
                           "\nthrow accumulator = ", mBallThrowAccumulator,
                           "\nmax balls = ", mMaxBalls,
                           "\nslomo factor = ", mSloMoFactor,
                           "\nair density = ", mAirDensity,
                           "\nthrow start pos = ", mThrowStartPos,
                           "\nball throw velocity = ", mThrowStartVelocity,
                           "\nball mass = ", mBallMass,
                           "\nball radius = ", mBallRadius,
                           "\nball path_length = ", mBallPathLength,
                           "\npaused = ", mPaused,
                           "\nshow throw lines = ", mShowLauncherLines,
                           "\nvector display type = ",
                           mVectorDisplayType == DisplayForce ? "forces"
                                                              : "accelerations",
                           "\nauto-pause on ground hit = ", mPauseOnGroundHit),
                x, y, sb::Color::White, line);
        line += 15;
        return line;
    }

    const Ball* Simulation::raycast(const Vec3& rayOrig, const Vec3& rayDir)
    {
        Vec3d orig = Vec3d(rayOrig);
        Vec3d dir = Vec3d(rayDir);
        dir = glm::normalize(dir);

        double intersection = std::numeric_limits<double>::infinity();
        Ball* ret = NULL;
        for (std::list<Ball*>::iterator it = mBalls.begin(); it != mBalls.end(); ++it)
        {
            Vec3d origToCenter(orig - (*it)->mPos);
            double b = -glm::dot(dir, origToCenter);
            double det = b * b - glm::dot(origToCenter, origToCenter) + (*it)->mRadius * (*it)->mRadius;

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
                ret = *it;
            }
        }

        return ret;
    }

    uint32_t Simulation::printBallParametersToScreen(const Ball* ball, float x, float y, uint32_t line)
    {
        using sb::utils::makeString;

        if (ball)
        {
            sb::String::print(makeString("velocity = ", ball->mVelocity.first,
                                         " (", ball->mVelocity.first.length(), ")"),
                              x, y, ColorVelocity, line++);
            sb::String::print(makeString("gravity = ", ball->mAccGravity.first,
                                         " (", ball->mAccGravity.first.length(), ")"),
                              x, y, ColorGravity, line++);
            sb::String::print(makeString("drag = ", ball->mAccDrag.first,
                                         " (", ball->mAccDrag.first.length(), ")"),
                              x, y, ColorDrag, line++);
            sb::String::print(makeString("wind = ", ball->mAccWind.first,
                                         " (", ball->mAccWind.first.length(), ")"),
                              x, y, ColorWind, line++);
            sb::String::print(makeString("buoyancy = ", ball->mAccBuoyancy.first,
                                         " (", ball->mAccBuoyancy.first.length(), ")"),
                              x, y, ColorBuoyancy, line++);
            sb::String::print(makeString("net = ", ball->mAccNet.first,
                                         " (", ball->mAccNet.first.length(), ")"),
                              x, y, ColorNet, line++);
            sb::String::print(makeString("distance covered = ",
                                         ball->mDistanceCovered),
                              x, y, ColorPath, line++);
            sb::String::print(makeString("mass = ", ball->mMass,
                                         "\nradius = ", ball->mRadius,
                                         "\narea = ", ball->mArea,
                                         "\nvolume = ", ball->mVolume,
                                         "\nhorizontal distance covered = ",
                                         ball->mHorizontalDistanceCovered,
                                         "\nenergy = ", ball->mTotalEnergy,
                                         "\ntime = ", ball->mTime,
                                         "\nTTL = ", ball->mTimeToLive),
                              x, y, sb::Color::White, line);
            line += 9;
        }
        else
            sb::String::print("no ball selected", x, y, sb::Color::White, line++);

        return line;
    }
}

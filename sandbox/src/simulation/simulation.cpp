#include "simulation.h"

#include "ball.h"
#include "../utils/stringUtils.h"
#include "../rendering/string.h"
#include "../utils/logger.h"

namespace Sim
{
    Simulation::Simulation(ESimType type):
        mThrowStartPos(Vec3d(0., 0., 0.)),
        mThrowStartVelocity(Vec3d(0., 0., 0.)),
        mThrowStartLine(Vec3(1.f, 1.f, 1.f), sb::Color(ColorThrow, 0.4f)),
        mGravityLine(Vec3(1.f, 1.f, 1.f), sb::Color(ColorGravity, 0.4f)),
        mWindVelocityLine(Vec3(1.f, 1.f, 1.f), sb::Color(ColorWind, 0.4f)),
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
        gLog.Info("starting simulation, type: %d\n", (int)type);

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

        SetThrowStart(Vec3d(0., 1., 0.), Vec3d(20., 20., 0.));
        SetGravity(Vec3d(0., -9.81, 0.));
        SetWind(Vec3d(0., 0., 0.));
    }

    Simulation::~Simulation()
    {
        for (std::list<Ball*>::iterator it = mBalls.begin(); it != mBalls.end(); ++it)
            delete (*it);
    }

    void Simulation::Update(float dt)
    {
        if (mPaused)
            return;

        if (mSloMoFactor > 0.f && (mSimType == SimContiniousThrow || mBalls.empty()))
        {
            mBallThrowAccumulator += dt;
            if (mBallThrowAccumulator >= mBallThrowDelay && mBalls.size() < mMaxBalls)
            {
                mBalls.push_back(new Ball(mThrowStartPos, mThrowStartVelocity, mBallMass, mBallRadius));
                mBallThrowAccumulator = 0.f;
            }
        }

        dt *= mSloMoFactor;

        for (std::list<Ball*>::iterator it = mBalls.begin(); it != mBalls.end(); ++it)
            if ((*it)->Update(dt, mGravity, mWindVelocity, mAirDensity, mBallPathLength, mVectorDisplayType == DisplayForce)) // returns true on collision with ground
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

    void Simulation::SetThrowStart(const Vec3d& pos, const Vec3d& v)
    {
        gLog.Info("simulation: throw_start set to %s\n", sb::StringUtils::ToString(v).c_str());

        mThrowStartLine.SetPosition((float)pos[0], (float)pos[1], (float)pos[2]);
        mThrowStartLine.SetScale((float)v[0], (float)v[1], (float)v[2]);
        mThrowStartPos = pos;
        mThrowStartVelocity = v;

        mGravityLine.AttachTo(&mThrowStartLine);
        mWindVelocityLine.AttachTo(&mThrowStartLine);
        mGravityLine.Update();
        mWindVelocityLine.Update();
    }

    void Simulation::SetGravity(const Vec3d& g)
    {
        gLog.Info("simulation: gravity set to %s\n", sb::StringUtils::ToString(g).c_str());

        mGravityLine.SetScale((float)g[0], (float)g[1], (float)g[2]);
        mGravity = g;
    }

    void Simulation::SetWind(const Vec3d& w)
    {
        gLog.Info("simulation: wind set to %s\n", sb::StringUtils::ToString(w).c_str());

        mWindVelocityLine.SetScale((float)w[0], (float)w[1], (float)w[2]);
        mWindVelocity = w;
    }

    void Simulation::DrawAll(sb::Renderer& renderer)
    {
        if (mShowLauncherLines)
        {
            renderer.Draw(mThrowStartLine);
            renderer.Draw(mGravityLine);
            renderer.Draw(mWindVelocityLine);
        }

        for (std::list<Ball*>::iterator it = mBalls.begin(); it != mBalls.end(); ++it)
            (*it)->DrawAll(renderer);
    }

    void Simulation::TogglePause()
    {
        mPaused = !mPaused;
    }

    void Simulation::Reset()
    {
        for (std::list<Ball*>::iterator it = mBalls.begin(); it != mBalls.end(); ++it)
            delete (*it);

        mBalls.clear();
    }

    // returns lines displayed
    uint32_t Simulation::PrintParametersToScreen(float x, float y, uint32_t line)
    {
        sb::String::Print("velocity = " + sb::StringUtils::ToString(mThrowStartVelocity) + " (" + sb::StringUtils::ToString(mThrowStartVelocity.length()) + ")", x, y, ColorVelocity, line++);
        sb::String::Print("gravity = " + sb::StringUtils::ToString(mGravity) + " (" + sb::StringUtils::ToString(mGravity.length()) + ")", x, y, ColorGravity, line++);
        sb::String::Print("drag", x, y, ColorDrag, line++);
        sb::String::Print("wind velocity = " + sb::StringUtils::ToString(mWindVelocity) + " (" + sb::StringUtils::ToString(mWindVelocity.length()) + ")", x, y, ColorWind, line++);
        sb::String::Print("buoyancy", x, y, ColorBuoyancy, line++);
        sb::String::Print("net", x, y, ColorNet, line++);
        sb::String::Print("trajectory", x, y, ColorPath, line++);
        // additional line
        sb::String::Print("simulation type = " + std::string(mSimType == SimSingleThrow ? "single ball" : "multiple balls") +
                      "\nthrow delay = " + sb::StringUtils::ToString(mBallThrowDelay) +
                      "\nthrow accumulator = " + sb::StringUtils::ToString(mBallThrowAccumulator) +
                      "\nmax balls = " + sb::StringUtils::ToString(mMaxBalls) +
                      "\nslomo factor = " + sb::StringUtils::ToString(mSloMoFactor) +
                      "\nair density = " + sb::StringUtils::ToString(mAirDensity) +
                      "\nthrow start pos = " + sb::StringUtils::ToString(mThrowStartPos) +
                      "\nball throw velocity = " + sb::StringUtils::ToString(mThrowStartVelocity) +
                      "\nball mass = " + sb::StringUtils::ToString(mBallMass) +
                      "\nball radius = " + sb::StringUtils::ToString(mBallRadius) +
                      "\nball path_length = " + sb::StringUtils::ToString(mBallPathLength) +
                      "\npaused = " + sb::StringUtils::ToString(mPaused) +
                      "\nshow throw lines = " + sb::StringUtils::ToString(mShowLauncherLines) +
                      "\nvector display type = " + (mVectorDisplayType == DisplayForce ? "forces" : "accelerations") +
                      "\nauto-pause on ground hit = " + sb::StringUtils::ToString(mPauseOnGroundHit),
                      x, y, sb::Color::White, line);
        line += 15;
        return line;
    }

    const Ball* Simulation::Raycast(const Vec3& rayOrig, const Vec3& rayDir)
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

    uint32_t Simulation::PrintBallParametersToScreen(const Ball* ball, float x, float y, uint32_t line)
    {
        if (ball)
        {
            sb::String::Print("velocity = " + sb::StringUtils::ToString(ball->mVelocity.first) + " (" + sb::StringUtils::ToString(ball->mVelocity.first.length()) + ")", x, y, ColorVelocity, line++);
            sb::String::Print("gravity = " + sb::StringUtils::ToString(ball->mAccGravity.first) + " (" + sb::StringUtils::ToString(ball->mAccGravity.first.length()) + ")", x, y, ColorGravity, line++);
            sb::String::Print("drag = " + sb::StringUtils::ToString(ball->mAccDrag.first) + " (" + sb::StringUtils::ToString(ball->mAccDrag.first.length()) + ")", x, y, ColorDrag, line++);
            sb::String::Print("wind = " + sb::StringUtils::ToString(ball->mAccWind.first) + " (" + sb::StringUtils::ToString(ball->mAccWind.first.length()) + ")", x, y, ColorWind, line++);
            sb::String::Print("buoyancy = " + sb::StringUtils::ToString(ball->mAccBuoyancy.first) + " (" + sb::StringUtils::ToString(ball->mAccBuoyancy.first.length()) + ")", x, y, ColorBuoyancy, line++);
            sb::String::Print("net = " + sb::StringUtils::ToString(ball->mAccNet.first) + " (" + sb::StringUtils::ToString(ball->mAccNet.first.length()) + ")", x, y, ColorNet, line++);
            sb::String::Print("distance covered = " + sb::StringUtils::ToString(ball->mDistanceCovered), x, y, ColorPath, line++);
            sb::String::Print("mass = " + sb::StringUtils::ToString(ball->mMass) +
                          "\nradius = " + sb::StringUtils::ToString(ball->mRadius) +
                          "\narea = " + sb::StringUtils::ToString(ball->mArea) +
                          "\nvolume = " + sb::StringUtils::ToString(ball->mVolume) +
                          "\nhorizontal distance covered = " + sb::StringUtils::ToString(ball->mHorizontalDistanceCovered) +
                          "\nenergy = " + sb::StringUtils::ToString(ball->mTotalEnergy) +
                          "\ntime = " + sb::StringUtils::ToString(ball->mTime) +
                          "\nTTL = " + sb::StringUtils::ToString(ball->mTimeToLive), x, y, sb::Color::White, line);
            line += 9;
        }
        else
            sb::String::Print("no ball selected", x, y, sb::Color::White, line++);

        return line;
    }
}

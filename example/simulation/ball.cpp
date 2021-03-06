#include "ball.h"

namespace Sim
{
    const sb::Color& ColorVelocity = sb::Color::Red;
    const sb::Color& ColorGravity = sb::Color::Blue;
    const sb::Color& ColorDrag = sb::Color::Cyan;
    const sb::Color& ColorWind = sb::Color::Green;
    const sb::Color& ColorBuoyancy = sb::Color(0.96f, 0.64f, 0.3f);
    const sb::Color& ColorNet = sb::Color::White;
    const sb::Color& ColorThrow = sb::Color::Yellow;
    const sb::Color& ColorPath = sb::Color(0.7f, 0.f, 0.7f);

    Ball::Ball(const sb::Vec3d& pos,
               const sb::Vec3d& velocity,
               double mass,
               double radius,
               const std::shared_ptr<sb::Shader>& modelShader,
               const std::shared_ptr<sb::Shader>& lineShader):
        mVelocity(sb::Vec3d(1., 1., 1.),
                  sb::Line(sb::Vec3(1.f, 1.f, 1.f),
                           sb::Color(ColorVelocity, 0.6f),
                           lineShader)),
        mAccGravity(sb::Vec3d(1., 1., 1.),
                    sb::Line(sb::Vec3(1.f, 1.f, 1.f),
                             sb::Color(ColorGravity, 0.6f),
                             lineShader)),
        mAccDrag(sb::Vec3d(1., 1., 1.),
                 sb::Line(sb::Vec3(1.f, 1.f, 1.f),
                          sb::Color(ColorDrag, 0.6f),
                          lineShader)),
        mAccWind(sb::Vec3d(1., 1., 1.),
                 sb::Line(sb::Vec3(1.f, 1.f, 1.f),
                          sb::Color(ColorWind, 0.6f),
                          lineShader)),
        mAccBuoyancy(sb::Vec3d(1., 1., 1.),
                     sb::Line(sb::Vec3(1.f, 1.f, 1.f),
                              sb::Color(ColorBuoyancy, 0.6f),
                              lineShader)),
        mAccNet(sb::Vec3d(1., 1., 1.),
                sb::Line(sb::Vec3(1.f, 1.f, 1.f),
                         sb::Color(ColorNet, 0.6f),
                         lineShader)),
        mMass(mass),
        mRadius(radius),
        mArea(PI * radius * radius),
        mVolume(4. / 3. * PI * radius * radius * radius),
        mDistanceCovered(0.),
        mHorizontalDistanceCovered(0.),
        mDeltaDistance(0.0),
        mTime(0.0),
        mTotalEnergy(0.0),
        mPos(pos),
        mModel(std::make_shared<sb::Model>(
                   "sphere.obj",
                   modelShader,
                   gResourceMgr.getTexture("blue_marble.jpg"))),
        mTimeToLive(5u),
        mPath(),
        mLineShader(lineShader)
    {
        mPath.push_back(pos);

        mModel->setPosition(0.f, 0.f, 0.f);
        mModel->setScale((float)(mRadius * 2.));

        set(mVelocity, velocity);
        set(mAccNet, mAccGravity.first + mAccDrag.first + mAccWind.first);

        attachLines();
    }

    void Ball::set(ColVec& what, const sb::Vec3d& value, bool scaleToForce)
    {
        what.first = value;
        if (scaleToForce)
            what.second.setScale(value * mMass);
        else
            what.second.setScale(value);
    }

    bool Ball::update(double dt, const sb::Vec3d& gravity, const sb::Vec3d& windVelocity, double fluidDensity, double maxPathLength, bool force)
    {
        mTime += dt;
        if (mTime < 0.0)
            return true;

        // updating position
        sb::Vec3d delta = mVelocity.first * dt;
        mPos += delta;

        static const double PATH_UPDATE_STEP = 0.5;
        mDeltaDistance += delta.length();

        if (mDeltaDistance > PATH_UPDATE_STEP)
        {
            mPath.push_back(mPos);
            if ((double)mPath.size() * PATH_UPDATE_STEP > maxPathLength)
                mPath.pop_front();

            mDeltaDistance -= PATH_UPDATE_STEP;
        }

        // total distance
        mDistanceCovered += delta.length();
        mHorizontalDistanceCovered += delta.xz().length();

        // to prevent falling under the ground
        if (mPos.y < mRadius)
        {
            mPos.y = mRadius;
            --mTimeToLive;
            // stop ball when touched ground
            set(mVelocity, sb::Vec3d(0., 0., 0.));
        }

        mModel->setPosition(mPos);

        // updating velocity
        set(mVelocity, mVelocity.first + mAccNet.first * dt, force);

        // updating energy
        if (mPos.y > mRadius)
            mTotalEnergy = mMass * (gravity.length() * mPos.y + mVelocity.first.dot(mVelocity.first) / 2.0);

        // accelerations:
        // gravity - constant
        set(mAccGravity, gravity, force);

        // drag
        // equation (en.wikipedia.org): Fd = 0.5 * (rho) * v^2 * Cd * A
        // Fd - drag force
        // rho - mass density of the fluid
        // v - velocity of the object, relative to the fluid
        // Cd - drag coefficient, 0.47 for sphere
        // A - reference area
        static double dragCoefficient = 0.47;
        sb::Vec3d newDragForce = -mVelocity.first.normalized() * 0.5
                             * fluidDensity
                             * mVelocity.first.dot(mVelocity.first)
                             * dragCoefficient
                             * mArea;

        sb::Vec3d dragForceStep = sb::Vec3d(newDragForce * dt);
        sb::Vec3d momentum = sb::Vec3d(mVelocity.first * mMass);
        if (dragForceStep.dot(dragForceStep) > momentum.dot(momentum))
        {
            // drag force too big!
            double length = sb::Vec3d(momentum / dt).length();
            set(mAccDrag, newDragForce.normalized() * length, force);
        }
        else
            set(mAccDrag, newDragForce / mMass, force);

        // wind
        sb::Vec3d newWindForce = (windVelocity != sb::Vec3d(0., 0., 0.)
                              ? windVelocity.normalized() * 0.5
                                * fluidDensity
                                * windVelocity.dot(windVelocity)
                                * dragCoefficient
                                * mArea
                              : sb::Vec3d(0., 0., 0.));
        set(mAccWind, newWindForce / mMass, force);

        // buoyancy
        sb::Vec3d buoyancyForce = fluidDensity * mVolume * -mAccGravity.first;
        set(mAccBuoyancy, buoyancyForce / mMass, force);

        // net
        set(mAccNet, mAccGravity.first + mAccDrag.first + mAccWind.first + mAccBuoyancy.first, force);

        mVelocity.second.update();
        mAccGravity.second.update();
        mAccDrag.second.update();
        mAccWind.second.update();
        mAccBuoyancy.second.update();
        mAccNet.second.update();

        return mTimeToLive == 0u;
    }

    void Ball::drawAll(sb::Renderer& r)
    {
        r.draw(*mModel);

        // temporarily disable depth testing, to make lines fully visible
        r.enableFeature(sb::Renderer::Feature::DepthTest, false);
        if (mPath.size() > 1)
        {
            static sb::Line line(sb::Vec3(1.f, 1.f, 1.f),
                                 sb::Color(ColorPath, 0.6f),
                                 mLineShader);

            std::list<sb::Vec3d>::iterator it, next;
            for (it = mPath.begin(), next = ++it; next != mPath.end(); it = next++)
            {
                line.setPosition(*it);
                line.setScale(*next - *it);
                r.draw(line);
            }
        }

        r.draw(mVelocity.second);

        r.draw(mAccGravity.second);
        r.draw(mAccDrag.second);
        r.draw(mAccWind.second);
        r.draw(mAccBuoyancy.second);
        r.draw(mAccNet.second);

        r.enableFeature(sb::Renderer::Feature::DepthTest);
    }

    void Ball::attachLines()
    {
        mVelocity.second.attachTo(mModel);
        mAccGravity.second.attachTo(mModel);
        mAccDrag.second.attachTo(mModel);
        mAccWind.second.attachTo(mModel);
        mAccBuoyancy.second.attachTo(mModel);
        mAccNet.second.attachTo(mModel);
    }
}

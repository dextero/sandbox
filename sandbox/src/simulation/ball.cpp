#include "ball.h"
#include "../utils/profiler.h"

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

	void Ball::AttachLines()
	{
		PROFILE();

		mVelocity.second.AttachTo(&mModel);
		mAccGravity.second.AttachTo(&mModel);
		mAccDrag.second.AttachTo(&mModel);
		mAccWind.second.AttachTo(&mModel);
		mAccBuoyancy.second.AttachTo(&mModel);
		mAccNet.second.AttachTo(&mModel);
	}

	Ball::Ball(const Vec3d& pos, const Vec3d& velocity, double mass, double radius):
		mVelocity(Vec3d(1., 1., 1.), sb::Line(Vec3(1.f, 1.f, 1.f), sb::Color(ColorVelocity, 0.6f))),
		mAccGravity(Vec3d(1., 1., 1.), sb::Line(Vec3(1.f, 1.f, 1.f), sb::Color(ColorGravity, 0.6f))),
		mAccDrag(Vec3d(1., 1., 1.), sb::Line(Vec3(1.f, 1.f, 1.f), sb::Color(ColorDrag, 0.6f))),
		mAccWind(Vec3d(1., 1., 1.), sb::Line(Vec3(1.f, 1.f, 1.f), sb::Color(ColorWind, 0.6f))),
		mAccBuoyancy(Vec3d(1., 1., 1.), sb::Line(Vec3(1.f, 1.f, 1.f), sb::Color(ColorBuoyancy, 0.6f))),
		mAccNet(Vec3d(1., 1., 1.), sb::Line(Vec3(1.f, 1.f, 1.f), sb::Color(ColorNet, 0.6f))),
		mMass(mass),
		mRadius(radius),
		mArea(PI * radius * radius),
		mVolume(4. / 3. * PI * radius * radius * radius),
		mDistanceCovered(0.),
		mHorizontalDistanceCovered(0.),
		mDeltaDistance(0.0),
		mTime(0.0),
		mPos(pos),
		mModel(L"sphere.obj"),
		mTimeToLive(5u)
	{
		PROFILE();

		mPath.push_back(pos);

		mModel.SetPosition(0.f, 0.f, 0.f);
		mModel.SetScale((float)(mRadius * 2.));

		Set(mVelocity, velocity);
		Set(mAccNet, mAccGravity.first + mAccDrag.first + mAccWind.first);

		AttachLines();
	}

	Ball::Ball(const Ball& copy)
	{
		PROFILE();

		*this = copy;

		AttachLines();
	}

	void Ball::Set(ColVec& what, const Vec3d& value, bool scaleToForce)
	{
		PROFILE();

		what.first = value;
		if (scaleToForce)
			what.second.SetScale(Vec3((float)(value[0] * mMass), (float)(value[1] * mMass), (float)(value[2] * mMass)));
		else
			what.second.SetScale(Vec3((float)value[0], (float)value[1], (float)value[2]));
	}

	bool Ball::Update(double dt, const Vec3d& gravity, const Vec3d& windVelocity, double fluidDensity, double maxPathLength, bool force)
	{
		PROFILE();

		mTime += dt;
		if (mTime < 0.0)
			return true;

		// updating position
		Vec3d delta = mVelocity.first * dt;
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
		mHorizontalDistanceCovered += sqrt(delta[0] * delta[0] + delta[2] * delta[2]);

		// to prevent falling under the ground
		if (mPos[1] < mRadius)
		{
			mPos[1] = mRadius;
			--mTimeToLive;
			// stop ball when touched ground
			Set(mVelocity, Vec3d(0., 0., 0.));
		}

		mModel.SetPosition((float)mPos[0], (float)mPos[1], (float)mPos[2]);

		// updating velocity
		Set(mVelocity, mVelocity.first + mAccNet.first * dt, force);

		// updating energy
		if (mPos[1] > mRadius)
			mTotalEnergy = mMass * (gravity.length() * mPos[1] + mVelocity.first.length_squared() / 2.0);

		// accelerations:
		// gravity - constant
		Set(mAccGravity, gravity, force);

		// drag
		// equation (en.wikipedia.org): Fd = 0.5 * (rho) * v^2 * Cd * A
		// Fd - drag force
		// rho - mass density of the fluid
		// v - velocity of the object, relative to the fluid
		// Cd - drag coefficient, 0.47 for sphere
		// A - reference area
		static double dragCoefficient = 0.47;
		Vec3d newDragForce = (-mVelocity.first.normalized()) * 0.5 * fluidDensity * mVelocity.first.length_squared() * dragCoefficient * mArea;

		if (Vec3d(newDragForce * dt).length_squared() > Vec3d(mVelocity.first * mMass).length_squared())
		{
			// drag force too big!
			double length = Vec3d(mVelocity.first * mMass / dt).length();
			Set(mAccDrag, newDragForce.normalized() * length, force);
		}
		else
			Set(mAccDrag, newDragForce / mMass, force);

		// wind
		Vec3d newWindForce = (windVelocity != Vec3d(0., 0., 0.) ? windVelocity.normalized() * 0.5 * fluidDensity * windVelocity.length_squared() * dragCoefficient * mArea : Vec3d(0., 0., 0.));
		Set(mAccWind, newWindForce / mMass, force);

		// buoyancy
		Vec3d buoyancyForce = fluidDensity * mVolume * -mAccGravity.first;
		Set(mAccBuoyancy, buoyancyForce / mMass, force);

		// net
		Set(mAccNet, mAccGravity.first + mAccDrag.first + mAccWind.first + mAccBuoyancy.first, force);

		mVelocity.second.Update();
		mAccGravity.second.Update();
		mAccDrag.second.Update();
		mAccWind.second.Update();
		mAccBuoyancy.second.Update();
		mAccNet.second.Update();

		return mTimeToLive == 0u;
	}

	void Ball::DrawAll(sb::Renderer& r)
	{
		PROFILE();

		r.Draw(mModel);

		// temporarily disable depth testing, to make lines fully visible
		r.EnableFeature(sb::Renderer::FeatureDepthTest, false);
		if (mPath.size() > 1)
		{
			static sb::Line line(Vec3(1.f, 1.f, 1.f), sb::Color(ColorPath, 0.6f));
			
			std::list<Vec3d>::iterator it, next;
			for (it = mPath.begin(), next = ++it; next != mPath.end(); it = next++)
			{
				line.SetPosition((float)(*it)[0], (float)(*it)[1], (float)(*it)[2]);
				line.SetScale((float)((*next)[0] - (*it)[0]), (float)((*next)[1] - (*it)[1]), (float)((*next)[2] - (*it)[2]));
				r.Draw(line);
			}
		}

		r.Draw(mVelocity.second);

		r.Draw(mAccGravity.second);
		r.Draw(mAccDrag.second);
		r.Draw(mAccWind.second);
		r.Draw(mAccBuoyancy.second);
		r.Draw(mAccNet.second);

		r.EnableFeature(sb::Renderer::FeatureDepthTest);
	}
}

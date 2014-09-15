#include "boids.h"
#include "utils/vector.h"
#include "utils/math.h"

using namespace sb;

namespace Sim {
namespace {
namespace boids {

inline Vec3 massRule(const Fish& fish,
                     const Vec3& shoalCenter)
{
    return (shoalCenter - fish.getPosition()) / 10000;
}

inline Vec3 notSoCloseRule(const Fish& fish,
                           const std::vector<Fish>& allFish)
{
    constexpr float MIN_DISTANCE = 1.5f;
    constexpr float MIN_DISTANCE_SQUARED = MIN_DISTANCE * MIN_DISTANCE;

    Vec3 closeness(0.f, 0.f, 0.f);
    const Vec3& pos = fish.getPosition();

    for (const Fish& other: allFish) {
        const Vec3& otherPos = other.getPosition();
        if (otherPos.distance2To(pos) < MIN_DISTANCE_SQUARED) {
            closeness -= (otherPos - pos) / 100.0f;
        }
    }

    return closeness;
}

inline Vec3 similarVelocityRule(const Fish& fish,
                                const Vec3& avgVelocity)
{
    return (avgVelocity - fish.getVelocity()) / 100;
}

inline Vec3 tendToPlace(const Fish& fish, Vec3 place)
{
    return (place - fish.getPosition()) / 10000;
}

inline Vec3 notSoFast(const Fish& fish,
                      float maxSpeedSquared)
{
    const Vec3& v = fish.getVelocity();

    if (v.dot(v) > maxSpeedSquared) {
        return -fish.getVelocity() / 2;
    }
    return Vec3(0.f, 0.f, 0.f);
}

inline Vec3 avoidPredator(const Fish& fish,
                          Vec3 predatorPosition)
{
    constexpr float ESCAPE_DISTANCE = 3.0f;
    constexpr float ESCAPE_DISTANCE_SQUARED = ESCAPE_DISTANCE * ESCAPE_DISTANCE;
    if (predatorPosition.distance2To(fish.getPosition()) < ESCAPE_DISTANCE_SQUARED) {
        return (fish.getPosition() - predatorPosition) * 0.005f;
    }

    return { 0.0f, 0.0f, 0.0f };
}

} // namespace boids

inline void updateVelocity(Fish& fish,
                           const std::vector<Fish>& allFish,
                           const Vec3& predatorPos,
                           const Vec3& shoalCenter,
                           const Vec3& avgVelocity)
{
    const Vec3 TARGET { 0.0f, 5.0f, 1.0f };
    const Vec3& v = fish.getVelocity();

    Vec3 newV = v;
    newV += boids::tendToPlace(fish, TARGET);
    newV += boids::massRule(fish, shoalCenter);
    newV += boids::notSoCloseRule(fish, allFish);
    newV += boids::similarVelocityRule(fish, avgVelocity);
    newV += boids::avoidPredator(fish, predatorPos);

    fish.setVelocity(newV);
}

inline void updatePosition(Fish& fish,
                           float dt)
{
    const Vec3& v = fish.getVelocity();
    const Vec3& pos = fish.getPosition();

    Vec3 currPos = pos + v * dt;

    if (currPos.y < 1.0f)  {
        if (v.y < 0.0f)  {
            fish.setVelocity(Vec3(v.x, 0.0f, v.z));
        }
    }

    fish.setPosition(currPos);
}

} // namespace

Boids::Boids(int size,
             const std::shared_ptr<Shader>& textureShader)
{
    for (int i = 0; i < size; ++i)
    {
        Fish fish("Crow/vorona.obj",
                  textureShader, gResourceMgr.getTexture("crow.tif"));

        fish.setPosition(math::random_float(-10.0f, 10.0f),
                         math::random_float(0.0f, 20.0f),
                         math::random_float(-10.0f, 10.0f));
        fish.setVelocity(math::random_float(-0.5f, 0.5f),
                         math::random_float(-0.5f, 0.5f),
                         math::random_float(-0.5f, 0.5f));
        fish.setScale(0.003f);
        shoalOfFish.push_back(fish);
    }
}

void Boids::update(float dt,
                   const Vec3& predatorPos)
{
    const Vec3 INITIAL_ORIENTATION { 0.f, 0.f, 1.f };

    Vec3 shoalCenter { 0.0f, 0.0f, 0.0f };
    Vec3 avgVelocity { 0.0f, 0.0f, 0.0f };

    for(const Fish& fish: shoalOfFish) {
        shoalCenter += fish.getPosition();
        avgVelocity += fish.getVelocity();
    }
    shoalCenter /= (float)shoalOfFish.size();
    avgVelocity /= (float)shoalOfFish.size();

    for (Fish& fish: shoalOfFish) {
        updateVelocity(fish, shoalOfFish,
                       predatorPos, shoalCenter, avgVelocity);
        updatePosition(fish, dt);

        const Vec3& v = fish.getVelocity();
        Vec3 rotationAxis = INITIAL_ORIENTATION.cross(v);
        float rotationAngle = acos(INITIAL_ORIENTATION.dot(v));

        if (!rotationAxis.isZero()) {
            fish.setRotation(rotationAxis, Radians(rotationAngle));
        }
    }
}

} // namespace Sim


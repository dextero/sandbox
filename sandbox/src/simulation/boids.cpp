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
    Vec3 delta = shoalCenter - fish.getPosition();
    float distanceSquared = delta.length2();
    return delta * exp(math::clamp(distanceSquared, 0.0f, 2.0f)) / 1000.0f;
}

inline Vec3 notSoCloseRule(const Fish& fish,
                           const std::vector<Fish>& allFish)
{
    constexpr float MIN_DISTANCE = 1.5f;
    constexpr float MIN_DISTANCE_SQUARED = MIN_DISTANCE * MIN_DISTANCE;
    constexpr float CLOSENESS_FACTOR = 1.0f;

    Vec3 closeness(0.f, 0.f, 0.f);
    const Vec3& pos = fish.getPosition();

    for (const Fish& other: allFish) {
        const Vec3& otherPos = other.getPosition();
        if (otherPos.distance2To(pos) < MIN_DISTANCE_SQUARED) {
            closeness -= (otherPos - pos) * CLOSENESS_FACTOR;
        }
    }

    return closeness;
}

inline Vec3 similarVelocityRule(const Fish& fish,
                                const Vec3& avgVelocity)
{
    constexpr float SIMILAR_VELOCITY_FACTOR = 0.01f;

    return (avgVelocity - fish.getVelocity()) * SIMILAR_VELOCITY_FACTOR;
}

inline Vec3 tendToPlace(const Fish& fish, Vec3 place)
{
    constexpr float TEND_SPEED_FACTOR = 0.0001f;

    return (place - fish.getPosition()) * TEND_SPEED_FACTOR;
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
    constexpr float ESCAPE_DISTANCE = 10.0f;
    constexpr float ESCAPE_DISTANCE_SQUARED = ESCAPE_DISTANCE * ESCAPE_DISTANCE;
    constexpr float ESCAPE_SPEED_FACTOR = 10.0f;

    Vec3 delta = fish.getPosition() - predatorPosition;
    float distance = delta.length2();
    if (distance < ESCAPE_DISTANCE_SQUARED) {
        return delta.normalized() * exp(-distance / 100.0f) * ESCAPE_SPEED_FACTOR;
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
    const float MAX_VELOCITY = 5.0f;
    const float MAX_VELOCITY_SQUARED = MAX_VELOCITY * MAX_VELOCITY;
    const Vec3& v = fish.getVelocity();

    Vec3 newV = v;
    newV += boids::tendToPlace(fish, TARGET);
    newV += boids::massRule(fish, shoalCenter);
    newV += boids::notSoFast(fish, MAX_VELOCITY_SQUARED);
    newV += boids::notSoCloseRule(fish, allFish);
    newV += boids::similarVelocityRule(fish, avgVelocity);
    newV += boids::avoidPredator(fish, predatorPos);

    fish.setVelocity(newV);
}

inline void updatePosition(Fish& fish,
                           float dt,
                           float minimalYPos)
{
    const Vec3& v = fish.getVelocity();
    const Vec3& pos = fish.getPosition();

    Vec3 currPos = pos + v * dt;

    if (currPos.y < minimalYPos)  {
        Vec3 newV = v;
        newV.y += exp(math::clamp(currPos.y - 2.0f, 0.0f, 2.0f));
        fish.setVelocity(newV);
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
        fish.setVelocity(math::random_float(-10.0f, 10.0f),
                         math::random_float(-10.0f, 10.0f),
                         math::random_float(-10.0f, 10.0f));
        fish.setScale(0.003f);
        shoalOfFish.push_back(fish);
    }
}

void Boids::update(float dt,
                   const Vec3& predatorPos,
                   float minimalYPos)
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

    currentPosition = shoalCenter;

    for (Fish& fish: shoalOfFish) {
        updateVelocity(fish, shoalOfFish,
                       predatorPos, shoalCenter, avgVelocity);
        updatePosition(fish, dt, minimalYPos);

        const Vec3& v = fish.getVelocity();
        if (v.isZero()) {
            continue;
        }

        Vec3 normalizedV = v.normalized();
        Vec3 rotationAxis = INITIAL_ORIENTATION.cross(normalizedV);
        float rotationAngle = acos(INITIAL_ORIENTATION.dot(normalizedV));

        if (!rotationAxis.isZero()) {
            fish.setRotation(rotationAxis, Radians(rotationAngle));
        }
    }
}

} // namespace Sim


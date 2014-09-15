#ifndef BOIDS_H
#define BOIDS_H

#include "window/window.h"
#include "rendering/fish.h"
#include "utils/types.h"

namespace Sim {

class Boids
{
public:
    std::vector<sb::Fish> shoalOfFish;

    Boids(int size,
          const std::shared_ptr<sb::Shader>& textureShader);

    void update(float dt,
                const Vec3& predatorPos);
};

} // namespace Sim

#endif // BOIDS_H

#ifndef BOIDS_H
#define BOIDS_H

#include <sandbox/window/window.h>
#include "fish.h"

namespace Sim
{

    class Boids
    {

    public:
    	std::vector<sb::Fish> shoalOfFish;

    	Boids(int size, std::shared_ptr<sb::Shader> textureShader);

    	sb::Vec3 massRule(sb::Fish fish);
    	sb::Vec3 notSoCloseRule(sb::Fish fish);
    	sb::Vec3 similarVelocityRule(sb::Fish fish);

    	sb::Vec3 tendToPlace(sb::Fish fish, sb::Vec3 place);
    	sb::Vec3 notSoFast(sb::Fish fish, float max_speed);
    	sb::Vec3 avoidPredator(sb::Fish fish, sb::Vec3 predator_position);

    	sb::Vec3 calculateVelocity(sb::Fish& fish, sb::Vec3 predator_position);
    	sb::Vec3 calculatePosition(sb::Fish fish);
    	double distance(sb::Vec3 A, sb::Vec3 B);

    	// void updateBoidsState();

    };
}


#endif // BOIDS_H

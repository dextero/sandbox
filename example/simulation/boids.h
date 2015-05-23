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
    	
    	Vec3 massRule(sb::Fish fish);
    	Vec3 notSoCloseRule(sb::Fish fish);
    	Vec3 similarVelocityRule(sb::Fish fish);

    	Vec3 tendToPlace(sb::Fish fish, Vec3 place);
    	Vec3 notSoFast(sb::Fish fish, float max_speed);
    	Vec3 avoidPredator(sb::Fish fish, Vec3 predator_position);

    	Vec3 calculateVelocity(sb::Fish& fish, Vec3 predator_position);
    	Vec3 calculatePosition(sb::Fish fish);
    	double distance(Vec3 A, Vec3 B);

    	// void updateBoidsState();

    };
}


#endif // BOIDS_H

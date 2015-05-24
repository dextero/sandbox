#include "boids.h"

namespace Sim
{

	Boids::Boids(int size, std::shared_ptr<sb::Shader> textureShader)
	{
	    srand((unsigned)time(0));
	    for (int i = 0; i < size; ++i)
	    {
	            sb::Fish fish("salamon.obj",
	                     textureShader, gResourceMgr.getTexture("salamon2.jpg"));
	            fish.setPosition(rand() % 20 - 10.0, rand() % 20 - 10.0, rand() % 20 - 10.0);
	            fish.setVelocity((rand() % 50 - 25)/50.0, (rand() % 50 - 25)/50.0, (rand() % 50 - 25)/50.0);
	            fish.setScale(0.8f);
	            shoalOfFish.push_back(fish);
	    }
	}

	sb::Vec3 Boids::massRule(sb::Fish fish)
	{
	    sb::Vec3 m_centrum(0.f,0.f, 0.f);

	    for(sb::Fish fish_it : shoalOfFish)
	    {
	            m_centrum = m_centrum + fish_it.getPosition();
	    }
	    m_centrum = m_centrum - fish.getPosition();
	    m_centrum = m_centrum / ( shoalOfFish.size() - 1.0);

	    return (m_centrum - fish.getPosition()) / 10000;
	}

	double Boids::distance(sb::Vec3 A, sb::Vec3 B)
	{
	    return sqrt( (A.x - B.x)*(A.x - B.x) + (A.y - B.y)*(A.y - B.y) + (A.z - B.z)*(A.z - B.z));
	}

	sb::Vec3 Boids::notSoCloseRule(sb::Fish fish)
	{
	    sb::Vec3 closeness_vector(0.f,0.f, 0.f);

	    for(sb::Fish fish_it : shoalOfFish)
	    {
	        if (distance(fish_it.getPosition(), fish.getPosition()) < 0.5) {
	            closeness_vector = closeness_vector - (fish_it.getPosition() - fish.getPosition())/100;
	        }
	    }

	    return closeness_vector;
	}

	sb::Vec3 Boids::similarVelocityRule(sb::Fish fish)
	{
	    sb::Vec3 perceived_velocity(0.f,0.f, 0.f);

	    for(sb::Fish fish_it : shoalOfFish)
	    {
	        perceived_velocity = perceived_velocity + fish_it.getVelocity();
	    }

	    perceived_velocity = perceived_velocity - fish.getVelocity();
	    perceived_velocity = perceived_velocity / ( shoalOfFish.size() - 1);

	    return (perceived_velocity - fish.getVelocity()) / 100;
	}

	sb::Vec3 Boids::tendToPlace(sb::Fish fish, sb::Vec3 place)
	{
	    return (place - fish.getPosition()) / 100000;
	}

	sb::Vec3 Boids::notSoFast(sb::Fish fish, float max_speed)
	{
	    if (distance(sb::Vec3(0.f, 0.f, 0.f), fish.getPosition()) > max_speed)
	    {
	        return -fish.getVelocity() / 2;
	    }
	    return sb::Vec3(0.f, 0.f, 0.f);
	}

	sb::Vec3 Boids::avoidPredator(sb::Fish fish, sb::Vec3 predator_position)
	{
	    sb::Vec3 avoid_predator_velocity(0.f,0.f, 0.f);
	    if (distance(predator_position, fish.getPosition()) < 3.0f)
	    {
	        return (fish.getPosition() - predator_position) * 0.005f;
	    }
	    return avoid_predator_velocity;
	}


	sb::Vec3 Boids::calculateVelocity(sb::Fish& fish, sb::Vec3 predator_position)
	{
	    sb::Vec3 before = {0.f, 0.f, 1.f};
	    sb::Vec3 after = fish.getVelocity() + tendToPlace(fish, sb::Vec3(0.f, 1.f, 1.f)) + massRule(fish)  + notSoCloseRule(fish) + similarVelocityRule(fish) + avoidPredator(fish, predator_position);
	    sb::Vec3 rotation_axis = before.cross(fish.getVelocity());
	    float rotation_angle = acos(before.dot(fish.getVelocity()));

	    if(! rotation_axis.isZero()){
	        fish.setRotation(rotation_axis, sb::Radians(rotation_angle));
	    }

	    return after;
	}

	// int conuter = 0;

	// void animateFish(sb::Fish& fish)
	// {
	    // conuter++;
	    // if (conuter % 7 == 0) {
	    //     sb::Vec3 source = fish.getRotationAxis();
	    //     sb::Vec3 dest = fish.getVelocity();
	    //     sb::Vec3 rotAxis = source.cross(dest).normalized();
	    //     float rotAngle = acos(source.normalized().dot(dest.normalized()));
	    //     fish.setRotation(rotAxis, Radians(rotAngle/8));
	    // }

	// }

	sb::Vec3 Boids::calculatePosition(sb::Fish fish)
	{
	    return fish.getVelocity() + fish.getPosition();
	}


	// void Boids::updateBoidsState()
	// {
	// 	for(sb::Fish &fish : shoalOfFish) {
	//         fish.setVelocity(calculateVelocity(fish, wnd.getCamera().getEye()));
	//         // animateFish(fish);
	// 	}

	//     for(sb::Fish &fish : shoalOfFish) {
	//         fish.setPosition(calculatePosition(fish));
	//         wnd.draw(fish);
	//     }
	// }

}

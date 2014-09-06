#ifndef FISH_H
#define FISH_H

#include "model.h"
#include <string>

namespace sb
{
    class Fish: public Model
    {
    public:
    	const Vec3& getVelocity() const;
	    void setVelocity(float v_x, float v_y, float v_z);
	    void setVelocity(const Vec3& fVel);
        
        Fish(const std::string& path,
              const std::shared_ptr<Shader>& shader,
              const std::shared_ptr<Texture>& texture = {});
	protected:
		Vec3 fVelocity;
    };
        
}

#endif //MODEL_H

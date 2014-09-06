#include "fish.h"

namespace sb
{
    Fish::Fish(const std::string& path,
                 const std::shared_ptr<Shader>& shader,
                 const std::shared_ptr<Texture>& texture):
        Model(path, shader, texture)
    {}
    
    const Vec3& Fish::getVelocity() const
    {
        return fVelocity;
    }
    
    void Fish::setVelocity(float v_x, float v_y, float v_z)
    {
        fVelocity = Vec3(v_x, v_y, v_z);
    }
    
    void Fish::setVelocity(const Vec3& fVel)
    {
        fVelocity = fVel;
    }

}

#include "terrain.h"
#include "../resources/resourceMgr.h"

namespace sb
{
    Terrain::Terrain(const std::string& heightmap,
                     const std::string& texture,
                     const std::shared_ptr<Shader>& shader):
        Drawable(ProjectionType::Perspective,
                 gResourceMgr.getTerrain(heightmap),
                 gResourceMgr.getTexture(texture),
                 shader),
        mImg(gResourceMgr.getImage(heightmap))
    {}

    float Terrain::getHeightAt(float x, float z) const
    {
        Vec3 scale = getScale();
        Vec3 position = getPosition();

        ssize_t imgX = (x - position.x) / scale.x + 0.5f;
        ssize_t imgZ = (z - position.z) / scale.z + 0.5f;

        uint32_t w = mImg->getWidth();
        uint32_t h = mImg->getHeight();

        imgX %= w;
        imgZ %= h;

        if (imgX < 0) {
            imgX += w;
            sbAssert(imgX >= 0, "huh?");
        }
        if (imgZ < 0) {
            imgZ += h;
            sbAssert(imgZ >= 0, "huh?");
        }

        size_t index = imgZ * w + imgX;
        float height = decodeHeight(((uint32_t*)mImg->getRGBAData())[index]);

        return height * scale.y + position.y;
    }
} // namespace sb

#include "terrain.h"
#include "../resources/resourceMgr.h"
#include "utils/math.h"

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

        float imgXfloat = (x - position.x) / scale.x;
        float imgZfloat = (z - position.z) / scale.z;

        ssize_t imgX = (ssize_t)imgXfloat;
        ssize_t imgZ = (ssize_t)imgZfloat;

        float xMod = fmod(imgXfloat, 1.0);
        float zMod = fmod(imgZfloat, 1.0);

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

        uint32_t* pixels = (uint32_t*)mImg->getRGBAData();
        size_t index = imgZ * w + imgX;

        float heights[2][2] {
            { 0.0f, decodeHeight(pixels[index + w]) },
            { decodeHeight(pixels[index + 1]), 0.0f }
        };

        if (xMod < 1.0f - zMod) {
            heights[0][0] = decodeHeight(pixels[index]);
            heights[1][1] = heights[0][1] + heights[1][0] - heights[0][0];
        } else {
            heights[1][1] = decodeHeight(pixels[index + w + 1]);
            heights[0][0] = heights[0][1] + heights[1][0] - heights[1][1];
        }

        float height = math::bilerp(xMod, zMod, heights[0][0], heights[1][0], heights[0][1], heights[1][1]);
        return height * scale.y + position.y - 0.1f;
    }
} // namespace sb

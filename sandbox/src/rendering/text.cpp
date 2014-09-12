#include "rendering/text.h"

#include "resources/mesh.h"
#include "resources/font.h"

#include "utils/stringUtils.h"

#include <vector>

namespace sb {
namespace {

std::shared_ptr<Mesh> makeMeshForText(const std::string& text,
                                      const std::shared_ptr<Font>& font)
{
    std::vector<Vec3> vertices;
    std::vector<Vec2> texcoords;
    std::vector<uint32_t> indices;

    vertices.reserve(text.size() * 4);
    indices.reserve(text.size() * 6);

    uint32_t x = 0;
    uint32_t y = 0;
    for (char c: text) {
        if (c == '\n') {
            x = 0;
            y += font->getLineHeightPixels();
            continue;
        }

        const Font::Letter& l = font->getLetter((uint8_t)c);
        //gLog.debug("letter: %c (%d), w = %u, h = %u, texcoords = %s",
                   //c, (int)c, l.widthPixels, l.heightPixels,
                   //lexical_cast<std::string>(l.texcoords).c_str());
        size_t idxBase = vertices.size();

        vertices.push_back(Vec3(x, y, 0.0f));
        vertices.push_back(Vec3(x + l.widthPixels, y, 0.0f));
        vertices.push_back(Vec3(x, y + l.heightPixels, 0.0f));
        vertices.push_back(Vec3(x + l.widthPixels, y + l.heightPixels, 0.0f));

        texcoords.push_back(l.texcoords.bottomLeft());
        texcoords.push_back(l.texcoords.bottomRight);
        texcoords.push_back(l.texcoords.topLeft);
        texcoords.push_back(l.texcoords.topRight());

        indices.push_back(idxBase);
        indices.push_back(idxBase + 2);
        indices.push_back(idxBase + 1);
        indices.push_back(idxBase + 1);
        indices.push_back(idxBase + 2);
        indices.push_back(idxBase + 3);

        x += l.widthPixels;
    }

    return std::make_shared<Mesh>(Mesh::Shape::Triangle,
                                  vertices, texcoords,
                                  std::vector<Color>(), std::vector<Vec3>(),
                                  std::vector<Vec3>(), std::vector<Vec3>(),
                                  indices, font->getTexture());
}

} // namespace

Text::Text(const std::string& text,
           const std::shared_ptr<Font>& font,
           const std::shared_ptr<Shader>& shader):
    Drawable(ProjectionType::Orthographic,
             makeMeshForText(text, font),
             font->getTexture(),
             shader),
    mFont(font)
{
}

} // namespace sb


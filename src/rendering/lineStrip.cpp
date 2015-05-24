#include <sandbox/rendering/lineStrip.h>

#include <sandbox/resources/mesh.h>
#include <sandbox/utils/math.h>

namespace sb
{
    LineStrip::LineStrip(const std::vector<Vec3>& vertices,
                         const Color& col,
                         const std::shared_ptr<Shader>& shader):
        Drawable(ProjectionType::Perspective,
                 std::make_shared<Mesh>(Mesh::Shape::LineStrip,
                                        vertices,
                                        std::vector<Vec2>(),
                                        std::vector<Color>(vertices.size(), col),
                                        std::vector<Vec3>(),
                                        math::range<uint32_t>(vertices.size()),
                                        nullptr),
                 nullptr,
                 shader)
    {}
} // namespace sb

#include "rectangle2d.hpp"

#include "graphics/renderer.hpp"

namespace Aporia
{
    Rectangle2D::Rectangle2D(float x, float y, float width, float height, Color color /* = Colors::White */, ShaderRef shader /* = 0u */)
        : Rectangle2D(glm::vec3{ x, y, 0.0f }, width, height, color, shader)
    {
    }

    Rectangle2D::Rectangle2D(glm::vec3 position, float width, float height, Color color /* = Colors::White */, ShaderRef shader /* = 0u */)
        : Entity(Transform2D{ position }, Rectangular{ width, height }, Color{ color }, ShaderRef{ shader ? shader : Renderer::default_shader })
    {
    }
}

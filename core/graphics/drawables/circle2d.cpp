#include "circle2d.hpp"

#include "graphics/renderer.hpp"

namespace Aporia
{
    Circle2D::Circle2D(float x, float y, float radius, Color color /* = Colors::Black */, ShaderRef shader /* = 0u */)
        : Circle2D(glm::vec3{ x, y, 0.0f }, radius, color, shader)
    {
    }

    Circle2D::Circle2D(glm::vec3 position, float radius, Color color /* = Colors::Black */, ShaderRef shader /* = 0u */)
        : Entity(Transform2D{ position }, Circular{ radius }, Color{ color }, ShaderRef{ shader ? shader : Renderer::default_shader })
    {
    }
}

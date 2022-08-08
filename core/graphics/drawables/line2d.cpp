#include "line2d.hpp"

#include "graphics/renderer.hpp"

namespace Aporia
{
    Line2D::Line2D(glm::vec3 p_1, glm::vec2 p_2, Color color /* = Colors::Black */, ShaderRef shader /* = 0 */)
        : Entity(Transform2D{ p_1, (p_2 - glm::vec2(p_1)) / 2.0f },
            Linear2D{ p_2 - glm::vec2(p_1) },
            Color{ color },
            ShaderRef{ shader ? shader : Renderer::default_shader }) {}
}

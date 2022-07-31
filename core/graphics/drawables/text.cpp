#include "text.hpp"

#include "graphics/renderer.hpp"

namespace Aporia
{
    Text::Text(glm::vec3 position, std::string caption, float size, const Font& font, Color color /* = Aporia::Colors::White */, ShaderRef shader /* = 0u */)
        : Entity(Transform2D{ .position = std::move(position), .scale = glm::vec2{ size } },
                    Color{ std::move(color) }, ShaderRef{ shader ? shader : Renderer::font_shader }),
            caption(std::move(caption)),
            font(font)
    {
    }
}

#include "text.hpp"

#include "font_manager.hpp"
#include "graphics/renderer.hpp"

namespace Aporia
{
    Text::Text()
        : font(FontManager::default_font) {}

    Text::Text(glm::vec3 position, std::string caption, float size, const Font& font, Color color /* = Colors::White */, ShaderRef shader /* = 0 */)
        : Entity(Transform2D{ .position = std::move(position), .scale = glm::vec2{ size } },
                    Color{ std::move(color) }, ShaderRef{ shader ? shader : Renderer::font_shader }),
            caption( std::move(caption) ),
            font(font) {}
}

#include "sprite.hpp"

#include "graphics/renderer.hpp"

namespace Aporia
{
    Sprite::Sprite(const Texture& texture, glm::vec3 position, Color color /* = Colors::White */, ShaderRef shader /* = 0u */)
        : Sprite(texture,
            std::move(position),
            glm::vec2{ texture.origin.width, texture.origin.height } * (texture.v - texture.u),
            std::move(color),
            shader)
    {
    }

    Sprite::Sprite(const Texture& texture, glm::vec3 position, glm::vec2 size, Color color /* = Colors::White */, ShaderRef shader /* = 0u */)
        : Entity(Transform2D{ std::move(position), size / 2.0f },
            Texture{ texture },
            Rectangular{ size.x, size.y },
            Color{ std::move(color) },
            ShaderRef{ shader ? shader : Renderer::default_shader })
    {
    }
}

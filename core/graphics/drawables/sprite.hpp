#pragma once

#include <utility>

#include <glm/vec3.hpp>

#include "entity.hpp"
#include "components/color.hpp"
#include "components/rectangular.hpp"
#include "components/texture.hpp"
#include "components/transform2d.hpp"
#include "graphics/shader.hpp"

namespace Aporia
{
    struct Sprite final : public Entity<Transform2D, Texture, Rectangular, Color, ShaderRef>
    {
        Sprite() = default;
        Sprite(const Texture& texture, glm::vec3 position, Color color = Colors::White, ShaderRef shader = 0);
        Sprite(const Texture& texture, glm::vec3 position, glm::vec2 size, Color color = Colors::White, ShaderRef shader = 0);
    };
}

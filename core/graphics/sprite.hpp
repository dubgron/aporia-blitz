#pragma once

#include <utility>

#include <glm/vec3.hpp>

#include "entity.hpp"
#include "components/color.hpp"
#include "components/rectangular.hpp"
#include "components/texture.hpp"
#include "components/transform2d.hpp"

namespace Aporia
{
    struct Sprite final : public Entity<Transform2D, Texture, Rectangular, Color>
    {
        Sprite() = default;

        Sprite(const Texture& texture, glm::vec3 position, Color color = Colors::White)
            : Entity(Transform2D{ std::move(position), (texture.v - texture.u) / 2.0f },
                     Texture{ texture },
                     Rectangular{ texture.v.x - texture.u.x, texture.v.y - texture.u.y },
                     Color{ std::move(color) }) {}

        Sprite(const Texture& texture, glm::vec3 position, glm::vec2 size, Color color = Colors::White)
            : Entity(Transform2D{ std::move(position), (texture.v - texture.u) / 2.0f },
                     Texture{ texture },
                     Rectangular{ size.x, size.y },
                     Color{ std::move(color) }) {}
    };
}

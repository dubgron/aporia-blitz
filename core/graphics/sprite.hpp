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
            : Entity(Transform2D{ std::move(position), { texture.width / 2.0f, texture.height / 2.0f } },
                     Texture{ texture },
                     Rectangular{ texture.width, texture.height },
                     Color{ std::move(color) }) {}
    };
}

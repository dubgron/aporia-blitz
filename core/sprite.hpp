#pragma once

#include <SFML/System/Vector2.hpp>

#include "entity.hpp"
#include "logger.hpp"
#include "components/texture.hpp"
#include "components/transform2d.hpp"

namespace Aporia
{
    struct Sprite final : public Entity<Transform2D, Texture>
    {
        Sprite() = default;

        Sprite(const Texture& texture, sf::Vector2f position = { 0.0f, 0.0f })
            : Entity(Transform2D{ position, 0.0, { position.x + texture.width / 2.0f, position.y + texture.height / 2.0f } }, Texture{ texture })
        {
        }
    };
}

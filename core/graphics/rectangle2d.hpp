#pragma once

#include <SFML/System/Vector2.hpp>

#include "entity.hpp"
#include "components/color.hpp"
#include "components/rectangular.hpp"
#include "components/transform2d.hpp"

namespace Aporia
{
    struct Rectangle2D final : public Entity<Transform2D, Rectangular, Color>
    {
        Rectangle2D() = default;

        Rectangle2D(sf::Vector2f position, float width, float height, Color color = Colors::Black)
            : Entity(Transform2D{ position }, Rectangular{ width, height }, Color{ color }) {}

        Rectangle2D(float x, float y, float width, float height, Color color = Colors::Black)
            : Rectangle2D(sf::Vector2f(x, y), width, height, color) {}
    };
}

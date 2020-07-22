#pragma once

#include <utility>

#include <SFML/System/Vector2.hpp>

#include "entity.hpp"
#include "components/color.hpp"
#include "components/linear2d.hpp"
#include "components/transform2d.hpp"

namespace Aporia
{
    struct Line2D final : public Entity<Transform2D, Linear2D, Color>
    {
        Line2D() = default;

        Line2D(sf::Vector2f p_1, sf::Vector2f p_2, Color color = Colors::Black)
            : Entity(Transform2D{ p_1, (p_2 - p_1) / 2.0f }, Linear2D{ p_2 - p_1 }, Color{ color }) {}
    };
}

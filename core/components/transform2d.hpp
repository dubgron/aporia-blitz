#pragma once

#include <SFML/System/Vector2.hpp>

namespace Aporia
{
    struct Transform2D final
    {
        sf::Vector2f position;

        sf::Vector2f origin;
        double rotation = 0.0;

        sf::Vector2f scale = { 1.0f, 1.0f };
    };
}

#pragma once

#include <SFML/System/Vector2.hpp>

namespace Aporia
{
    struct Transform2D final
    {
        sf::Vector2f position;

        double rotation;
        sf::Vector2f origin;
    };
}

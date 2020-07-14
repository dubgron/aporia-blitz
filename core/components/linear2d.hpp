#pragma once

#include <utility>

#include <SFML/System/Vector2.hpp>

namespace Aporia
{
    struct Linear2D final
    {
        std::pair<sf::Vector2f, sf::Vector2f> vertecies;
    };
}

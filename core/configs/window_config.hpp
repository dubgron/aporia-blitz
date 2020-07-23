#pragma once

#include <string>

#include <SFML/System/Vector2.hpp>

namespace Aporia
{
    struct WindowConfig final
    {
        std::string title;
        sf::Vector2i position;
        unsigned int width = 0u;
        unsigned int height = 0u;
        bool vsync = true;
    };
}

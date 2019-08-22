#pragma once

#include <string>
#include <SFML/System/Vector2.hpp>

#include "platform.hpp"

namespace Aporia
{
    struct APORIA_API WindowConfig
    {
        std::string title = "";
        sf::Vector2i position = { 0, 0 };
        unsigned int width = 0;
        unsigned int height = 0;

        WindowConfig(std::string title, sf::Vector2i position, unsigned int width, unsigned int height)
            : title(std::move(title)), position(std::move(position)), width(width), height(height) {}
    };
}

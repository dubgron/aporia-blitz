#pragma once

#include <string>
#include <vector>
#include <iostream>

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
            : title(std::move(title)), position(std::move(position)), width(width), height(height) { }
    };

    struct APORIA_API Config
    {
        Config() = default;
        Config(const std::string& config);

        std::vector<WindowConfig> window_data;
    };
}

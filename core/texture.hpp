#pragma once

#include <memory>

#include <SFML/Graphics/Texture.hpp>

#include "logger.hpp"
#include "platform.hpp"

namespace Aporia
{
    struct APORIA_API Texture
    {
        unsigned int x, y;
        unsigned width, height;

        std::shared_ptr<sf::Texture> origin;

        Texture(unsigned int x, unsigned int y, unsigned int width, unsigned int height, std::shared_ptr<sf::Texture> origin)
            : x(x), y(y), width(width), height(height), origin(origin) {};
    };
}

#pragma once

#include <memory>

#include <SFML/Graphics/Texture.hpp>

namespace Aporia
{
    struct Texture final
    {
        unsigned int x, y;
        unsigned width, height;

        std::shared_ptr<sf::Texture> origin;

        Texture(unsigned int x, unsigned int y, unsigned int width, unsigned int height, const std::shared_ptr<sf::Texture>& origin)
            : x(x), y(y), width(width), height(height), origin(origin) {};
    };
}

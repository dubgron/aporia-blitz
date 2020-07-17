#pragma once

#include <memory>

#include <SFML/Graphics/Texture.hpp>

namespace Aporia
{
    struct Texture final
    {
        unsigned int x, y;
        float width, height;

        std::shared_ptr<sf::Texture> origin;
    };
}

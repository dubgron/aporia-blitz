#pragma once

#include <cstdint>
#include <memory>

#include <SFML/Graphics/Texture.hpp>

namespace Aporia
{
    struct Texture final
    {
        uint32_t x = 0;
        uint32_t y = 0;
        float width = 0.0f;
        float height = 0.0f;

        std::shared_ptr<sf::Texture> origin;
    };
}

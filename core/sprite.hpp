#pragma once

#include <memory>

#include <SFML/System/Vector2.hpp>

#include "logger.hpp"
#include "platform.hpp"
#include "texture.hpp"

namespace Aporia
{
    class APORIA_API Sprite final
    {
    public:
        Sprite(const std::shared_ptr<Logger>& logger, const std::shared_ptr<Texture>& texture, sf::Vector2f position)
            :_logger(logger), _texture(texture), _position(position) {};

        sf::Vector2f get_position() const
        {
            return _position;
        }

        const std::shared_ptr<Texture>& get_texture() const
        {
            return _texture;
        }

    private:
        std::shared_ptr<Logger> _logger;
        std::shared_ptr<Texture> _texture;
        sf::Vector2f _position;
    };
}
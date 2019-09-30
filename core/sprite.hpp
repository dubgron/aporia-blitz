#pragma once

#include <memory>

#include <SFML/System/Vector2.hpp>

#include "logger.hpp"
#include "platform.hpp"
#include "texture.hpp"

namespace Aporia
{
    enum class Origin
    {
        Center,
        Left,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    class APORIA_API Sprite final
    {
    public:
        Sprite(const std::shared_ptr<Logger>& logger, const std::shared_ptr<Texture>& texture, sf::Vector2f position)
            :_logger(logger), _texture(texture), _position(position) {};

        sf::Vector2f get_position() const;
        const sf::Vector2f& get_origin() const;

        const std::shared_ptr<Texture>& get_texture() const;

        void move(float x, float y);
        void move(const sf::Vector2f& pos);

        void set_origin(sf::Vector2f origin);
        void set_origin(Origin origin);

    private:
        std::shared_ptr<Logger> _logger;
        std::shared_ptr<Texture> _texture;
        sf::Vector2f _position;
        sf::Vector2f _origin;
    };
}
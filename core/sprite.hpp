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
        Sprite(const std::shared_ptr<Texture>& texture, sf::Vector2f position, Origin origin = Origin::Center, double rotation = 0);
        Sprite(const std::shared_ptr<Texture>& texture, sf::Vector2f position, sf::Vector2f origin, double rotation = 0);

        void move(float x, float y);
        void move(const sf::Vector2f& pos);
        void rotate(double degree);

        void set_origin(sf::Vector2f origin);
        void set_origin(Origin origin);
        void set_rotation(double degree);

        const std::shared_ptr<Texture>& get_texture() const;
        const sf::Vector2f& get_position() const;
        const sf::Vector2f& get_origin() const;
        double get_rotation() const;

        sf::Color color = sf::Color(0xffffffff);

    private:
        std::shared_ptr<Texture> _texture;
        sf::Vector2f _position;
        sf::Vector2f _origin;
        double _rotation;
    };
}
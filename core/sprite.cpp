#include "sprite.hpp"

#include <cmath>

inline constexpr double ARI_PI = 3.14159265359;

namespace Aporia
{
    Sprite::Sprite(const std::shared_ptr<Texture>& texture, sf::Vector2f position, Origin origin, double rotation)
        : _texture(texture), _position(position), _rotation(rotation)
    {
        set_origin(origin);
    }

    Sprite::Sprite(const std::shared_ptr<Texture>& texture, sf::Vector2f position, sf::Vector2f origin, double rotation)
        : _texture(texture), _position(position), _origin(origin), _rotation(rotation)
    {
    }

    void Sprite::move(float x, float y)
    {
        move(sf::Vector2f(x, y));
    }

    void Sprite::move(const sf::Vector2f& pos)
    {
        _position += pos;
    }

    void Sprite::rotate(double degree)
    {
        double radians = _rotation + (degree * ARI_PI / 180.0);
        _rotation = std::fmod(radians, 2 * ARI_PI);
    }

    void Sprite::set_origin(sf::Vector2f origin)
    {
        _origin = origin;
    }

    void Sprite::set_origin(Origin origin)
    {
        switch (origin)
        {
        case Origin::Center:
            set_origin(sf::Vector2f(_texture->width / 2, _texture->height / 2));
            break;
        case Origin::Left:
            set_origin(sf::Vector2f(0, _texture->height / 2));
            break;
        case Origin::Right:
            set_origin(sf::Vector2f(_texture->width, _texture->height / 2));
            break;
        case Origin::Top:
            set_origin(sf::Vector2f(_texture->width / 2, 0));
            break;
        case Origin::Bottom:
            set_origin(sf::Vector2f(_texture->width / 2, _texture->height));
            break;
        case Origin::TopLeft:
            set_origin(sf::Vector2f(0, 0));
            break;
        case Origin::TopRight:
            set_origin(sf::Vector2f(_texture->width, 0));
            break;
        case Origin::BottomLeft:
            set_origin(sf::Vector2f(0, _texture->height));
            break;
        case Origin::BottomRight:
            set_origin(sf::Vector2f(_texture->width, _texture->height));
            break;
        }
    }

    void Sprite::set_rotation(double degree)
    {
        double radians = degree * ARI_PI / 180.0;
        _rotation = std::fmod(radians, 2 * ARI_PI);
    }

    const sf::Vector2f& Sprite::get_position() const
    {
        return _position;
    }

    const std::shared_ptr<Texture>& Sprite::get_texture() const
    {
        return _texture;
    }

    const sf::Vector2f& Sprite::get_origin() const
    {
        return _origin;
    }

    double Sprite::get_rotation() const
    {
        return _rotation;
    }
}
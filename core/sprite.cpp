#include "sprite.hpp"

namespace Aporia
{
    sf::Vector2f Sprite::get_position() const
    {
        return _position;
    }

    const std::shared_ptr<Texture>& Sprite::get_texture() const
    {
        return _texture;
    }

    void Sprite::move(float x, float y)
    {
        move(sf::Vector2f(x, y));
    }

    void Sprite::move(const sf::Vector2f& pos)
    {
        _position += pos;
    }
}
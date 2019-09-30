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
}
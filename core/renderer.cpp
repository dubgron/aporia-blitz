#include "renderer.hpp"

#include <array>
#include <cmath>
#include <functional>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Vector2.hpp>

#include "event.hpp"

namespace Aporia
{
    Renderer::Renderer(const std::shared_ptr<Logger>& logger, const std::shared_ptr<EventManager>& event_manager)
        : _logger(logger)
    {
        using namespace std::placeholders;

        event_manager->add_listener<DrawSpriteEvent>(std::bind(&Renderer::draw, this, _1));
    }

    void Renderer::draw(const Sprite& sprite)
    {
        std::array<sf::Vertex, 4> vertecies;

        const std::shared_ptr<Texture>& texture = sprite.get_texture();
        const sf::Vector2f& position = sprite.get_position();
        const sf::Vector2f& origin = position + sprite.get_origin();
        double rotation = sprite.get_rotation();

        sf::Vector2f pos = position - origin;
        double sin = std::sin(rotation);
        double cos = std::cos(rotation);

        auto rotate = [=](float x, float y)
        {
            return sf::Vector2f(x * cos - y * sin, x * sin + y * cos);
        };

        vertecies[0].position = rotate(pos.x, pos.y) + origin;
        vertecies[1].position = rotate(pos.x + texture->width, pos.y) + origin;
        vertecies[2].position = rotate(pos.x + texture->width, pos.y + texture->height) + origin;
        vertecies[3].position = rotate(pos.x, pos.y + texture->height) + origin;

        vertecies[0].texCoords = sf::Vector2f(texture->x, texture->y);
        vertecies[1].texCoords = sf::Vector2f(texture->x + texture->width, texture->y);
        vertecies[2].texCoords = sf::Vector2f(texture->x + texture->width, texture->y + texture->height);
        vertecies[3].texCoords = sf::Vector2f(texture->x, texture->y + texture->height);

        if (_queue.find(texture->origin) == _queue.end())
            _queue.try_emplace(texture->origin, sf::Quads, _sprites);

        _queue[texture->origin].add(vertecies);
    }

    void Renderer::render(Window& window)
    {
        for (auto& [texture, vertex_array] : _queue)
        {
            sf::RenderStates states(texture.get());

            window.draw(vertex_array, states);
            vertex_array.clear();
        }
    }
}

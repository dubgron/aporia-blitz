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
    Renderer::Renderer(Logger& logger, EventManager& event_manager)
        : _logger(logger)
    {
        using namespace std::placeholders;

        event_manager.add_listener<DrawSpriteEvent>(std::bind(&Renderer::draw, this, _1));
    }

    void Renderer::draw(const Sprite& sprite)
    {
        std::array<sf::Vertex, 4> vertecies;

        const std::shared_ptr<Texture>& texture = sprite.get_texture();
        const sf::Vector2f& position = sprite.get_position();
        const sf::Vector2f& origin = position + sprite.get_origin();
        double rotation = sprite.get_rotation();
        bool flip_x = sprite.get_flip_x();
        bool flip_y = sprite.get_flip_y();

        double sin = std::sin(rotation);
        double cos = std::cos(rotation);

        auto rotate = [=](const sf::Vector2f& vec)
        {
            return sf::Vector2f(vec.x * cos - vec.y * sin, vec.x * sin + vec.y * cos);
        };

        vertecies[0].position = sf::Vector2f(position.x, position.y + texture->height);
        vertecies[1].position = sf::Vector2f(position.x + texture->width, position.y + texture->height);
        vertecies[2].position = sf::Vector2f(position.x + texture->width, position.y);
        vertecies[3].position = sf::Vector2f(position.x, position.y);

        vertecies[0].texCoords = sf::Vector2f(texture->x, texture->y);
        vertecies[1].texCoords = sf::Vector2f(texture->x + texture->width, texture->y);
        vertecies[2].texCoords = sf::Vector2f(texture->x + texture->width, texture->y + texture->height);
        vertecies[3].texCoords = sf::Vector2f(texture->x, texture->y + texture->height);

        for (auto& vertex : vertecies)
        {
            sf::Vector2f& v_pos = vertex.position;

            sf::Vector2f proj = v_pos;
            if (flip_x)
                proj.x = origin.x;

            if (flip_y)
                proj.y = origin.y;

            v_pos = 2.0f * proj - v_pos;
            v_pos = rotate(v_pos - origin) + origin;
        }

        if (_queue.find(texture->origin) == _queue.end())
            _queue.try_emplace(texture->origin, sf::Quads, _sprites);

        _queue[texture->origin].add(vertecies);
    }

    void Renderer::render(Window& window, Camera& camera)
    {
        for (auto& [texture, vertex_array] : _queue)
        {
            sf::RenderStates states(texture.get());
            states.transform = camera.get_view_projection_matrix();

            window.draw(vertex_array, states);
            vertex_array.clear();
        }
    }
}

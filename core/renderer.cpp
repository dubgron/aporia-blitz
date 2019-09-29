#include "renderer.hpp"

#include <array>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Vector2.hpp>

namespace Aporia
{
    Renderer::Renderer(const std::shared_ptr<Logger>& logger)
        : _logger(logger)
    {
    }

    void Renderer::draw(const Sprite& sprite)
    {
        std::array<sf::Vertex, 4> vertecies;
        const std::shared_ptr<Texture>& texture = sprite.get_texture();
        sf::Vector2f position = sprite.get_position();

        vertecies[0].position = position;
        vertecies[1].position = position + sf::Vector2f(texture->width, 0);
        vertecies[2].position = position + sf::Vector2f(texture->width, texture->height);
        vertecies[3].position = position + sf::Vector2f(0, texture->height);

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

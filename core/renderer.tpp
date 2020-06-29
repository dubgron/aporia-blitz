#pragma once


namespace Aporia
{
    template<typename T, std::enable_if_t<has_types_v<typename T::Components, Transform2D, Texture>, int>>
    void Renderer::draw(const T& entity)
    {
        const Transform2D& transform = entity.get_component<Transform2D>();
        const Texture& texture = entity.get_component<Texture>();

        std::array<sf::Vertex, 4> vertecies;

        const sf::Vector2f& position = transform.position;
        const sf::Vector2f& origin = position + transform.origin;
        double rotation = transform.rotation;

        double sin = std::sin(rotation);
        double cos = std::cos(rotation);

        auto rotate = [=](const sf::Vector2f& vec)
        {
            return sf::Vector2f(vec.x * cos - vec.y * sin, vec.x * sin + vec.y * cos);
        };

        vertecies[0].position = sf::Vector2f(position.x, position.y + texture.height);
        vertecies[1].position = sf::Vector2f(position.x + texture.width, position.y + texture.height);
        vertecies[2].position = sf::Vector2f(position.x + texture.width, position.y);
        vertecies[3].position = sf::Vector2f(position.x, position.y);

        vertecies[0].texCoords = sf::Vector2f(texture.x, texture.y);
        vertecies[1].texCoords = sf::Vector2f(texture.x + texture.width, texture.y);
        vertecies[2].texCoords = sf::Vector2f(texture.x + texture.width, texture.y + texture.height);
        vertecies[3].texCoords = sf::Vector2f(texture.x, texture.y + texture.height);

        if (rotation)
            for (auto& vertex : vertecies)
                vertex.position = rotate(vertex.position - origin) + origin;

        if (_queue.find(texture.origin) == _queue.end())
            _queue.try_emplace(texture.origin, sf::Quads, _sprites);

        _queue[texture.origin].add(vertecies);
    }
}

#pragma once

#include <cmath>
#include <utility>

#include <SFML/System/Vector2.hpp>

#include "utils/math.hpp"

namespace Aporia
{
    template<typename T, std::enable_if_t<has_type_v<typename T::Components, Rectangular>, int>>
    void Renderer::draw(const T& entity)
    {
        const Rectangular& rectangular = entity.get_component<Rectangular>();
        std::array<sf::Vertex, 4> vertecies;

        vertecies[0].position = sf::Vector2f(0.0f, 0.0f);
        vertecies[1].position = sf::Vector2f(rectangular.width, 0.0f);
        vertecies[2].position = sf::Vector2f(rectangular.width, rectangular.height);
        vertecies[3].position = sf::Vector2f(0.0f, rectangular.height);

        if constexpr (has_type_v<typename T::Components, Color>)
            _apply_color(vertecies, entity.get_component<Color>());

        if constexpr (has_type_v<typename T::Components, Transform2D>)
            _apply_transform2d(vertecies, entity.get_component<Transform2D>());

        if constexpr (has_type_v<typename T::Components, Texture>)
        {
            const Texture& texture = entity.get_component<Texture>();
            _apply_texture(vertecies, texture);

            if (_textured_queue.find(texture.origin) == _textured_queue.end())
                _textured_queue.try_emplace(texture.origin);

            _textured_queue[texture.origin].add(std::move(vertecies));
        }
        else
            _quad_queue.add(std::move(vertecies));
    }

    template<typename T, std::enable_if_t<has_type_v<typename T::Components, Circular>, int>>
    void Renderer::draw(const T& entity)
    {
        const Circular& circular = entity.get_component<Circular>();
        std::array<sf::Vertex, circular.point_count + 2> vertecies;

        for (unsigned int i = 1; i < circular.point_count + 2; ++i)
        {
            double angle = 2.0 * M_PI * (i - 1u) / circular.point_count;
            vertecies[i].position = sf::Vector2f(std::cos(angle), std::sin(angle)) * circular.radius;
        }

        if constexpr (has_type_v<typename T::Components, Color>)
            _apply_color(vertecies, entity.get_component<Color>());

        if constexpr (has_type_v<typename T::Components, Transform2D>)
            _apply_transform2d(vertecies, entity.get_component<Transform2D>());

        _circle_queue.add(std::move(vertecies));
    }

    template<typename T, std::enable_if_t<has_type_v<typename T::Components, Linear2D>, int>>
    void Renderer::draw(const T& entity)
    {
        const Linear2D& linear2d = entity.get_component<Linear2D>();
        std::array<sf::Vertex, 2> vertecies;

        vertecies[0] = linear2d.vertecies.first;
        vertecies[1] = linear2d.vertecies.second;

        if constexpr (has_type_v<typename T::Components, Color>)
            _apply_color(vertecies, entity.get_component<Color>());

        if constexpr (has_type_v<typename T::Components, Transform2D>)
            _apply_transform2d(vertecies, entity.get_component<Transform2D>());

        _line_queue.add(std::move(vertecies));
    }

    template<std::size_t N>
    void Renderer::_apply_color(std::array<sf::Vertex, N>& vertecies, const Color& color)
    {
        for (auto& vertex : vertecies)
            vertex.color = sf::Color(color.r, color.g, color.b, color.a);
    }

    template<std::size_t N>
    void Renderer::_apply_transform2d(std::array<sf::Vertex, N>& vertecies, const Transform2D& transform)
    {
        for (auto& vertex : vertecies)
        {
            vertex.position = transform * vertex.position;
        }
    }

    template<std::size_t N>
    void Renderer::_apply_texture(std::array<sf::Vertex, N>& vertecies, const Texture& texture)
    {
        vertecies[0].texCoords = sf::Vector2f(texture.x, texture.y + texture.height);
        vertecies[1].texCoords = sf::Vector2f(texture.x + texture.width, texture.y + texture.height);
        vertecies[2].texCoords = sf::Vector2f(texture.x + texture.width, texture.y);
        vertecies[3].texCoords = sf::Vector2f(texture.x, texture.y);
    }
}

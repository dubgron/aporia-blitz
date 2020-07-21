#pragma once

#include <array>
#include <cstddef>
#include <utility>
#include <vector>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>

namespace Aporia
{
    using IndexBuffer = sf::PrimitiveType;

    template<IndexBuffer T, std::size_t N>
    class VertexArray final : public sf::Drawable
    {
    public:
        VertexArray()
        {
            _vertices.reserve(N);
        }

        template<std::size_t Size>
        void add(std::array<sf::Vertex, Size>&& vertices)
        {
            for (sf::Vertex& vertex : vertices)
                _vertices.emplace_back(std::move(vertex));
        }

        void clear()
        {
            _vertices.clear();
        }

        std::size_t size() const
        {
            return _vertices.size();
        }

    private:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override
        {
            if (!_vertices.empty())
                target.draw(&_vertices[0], _vertices.size(), T, states);
        }

        std::vector<sf::Vertex> _vertices;
    };
}

#include "vertex_array.hpp"

#include <iostream>

namespace Aporia
{
    VertexArray::VertexArray(sf::PrimitiveType primitive_type, size_t size)
        : _primitive_type(primitive_type)
    {
        _vertices.reserve(size);
    }

    void VertexArray::add(const std::array<sf::Vertex, 4>& vertecies)
    {
        for (const auto& vertex : vertecies)
        {
            _vertices.push_back(std::move(vertex));
        }
    }

    void VertexArray::clear()
    {
        _vertices.clear();
    }

    size_t VertexArray::size() const
    {
        return _vertices.size();
    }

    void VertexArray::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if (!_vertices.empty())
        {
            target.draw(&_vertices[0], _vertices.size(), _primitive_type, states);
        }
    }
}

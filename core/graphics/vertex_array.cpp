#include "vertex_array.hpp"

namespace Aporia
{
    VertexArray::VertexArray(size_t vertex_count, size_t index_count)
    {
        glGenVertexArrays(1, &_id);

        if (vertex_count == 4 && index_count == 6)
        {
            _mode = GL_TRIANGLES;
        }
        else if (vertex_count == 2 && index_count == 2)
        {
            _mode = GL_LINES;
        }
    }

    VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
    {
        _id = other._id;
        _mode = other._mode;
        _vertex_buffer = std::move(other._vertex_buffer);
        _index_buffer = std::move(other._index_buffer);

        other._id = 0;

        return *this;
    }

    VertexArray::~VertexArray()
    {
        glDeleteVertexArrays(1, &_id);
    }

    void VertexArray::bind() const
    {
        glBindVertexArray(_id);
    }

    void VertexArray::unbind() const
    {
        glBindVertexArray(0);
    }

    void VertexArray::set_vertex_buffer(VertexBuffer&& vertex_buffer)
    {
        _vertex_buffer = std::move(vertex_buffer);
    }

    void VertexArray::set_index_buffer(IndexBuffer&& index_buffer)
    {
        _index_buffer = std::move(index_buffer);
    }

    void VertexArray::render()
    {
        _vertex_buffer.flush();

        this->bind();
        _index_buffer.bind();

        const uint32_t count = static_cast<uint32_t>(_vertex_buffer.size() / _vertex_buffer.count() * _index_buffer.count());
        glDrawElements(_mode, count, GL_UNSIGNED_INT, nullptr);

        _index_buffer.unbind();
        this->unbind();

        _vertex_buffer.clear();
    }
}

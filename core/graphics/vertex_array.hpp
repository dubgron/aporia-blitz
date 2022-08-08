#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "graphics/index_buffer.hpp"
#include "graphics/opengl.hpp"
#include "graphics/vertex_buffer.hpp"

namespace Aporia
{
    class VertexArray final
    {
    public:
        VertexArray(size_t vertex_count, size_t index_count)
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

        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;

        VertexArray(VertexArray&&) = default;
        VertexArray& operator=(VertexArray&& other) noexcept
        {
            this->_id               = other._id;
            this->_mode             = other._mode;
            this->_vertex_buffer    = std::move(other._vertex_buffer);
            this->_index_buffer     = std::move(other._index_buffer);

            other._id = 0;

            return *this;
        }

        ~VertexArray()
        {
            glDeleteVertexArrays(1, &_id);
        }

        void bind() const
        {
            glBindVertexArray(_id);
        }

        void unbind() const
        {
            glBindVertexArray(0);
        }

        void set_vertex_buffer(VertexBuffer&& vertex_buffer)
        {
            _vertex_buffer = std::move(vertex_buffer);
        }

        void set_index_buffer(IndexBuffer&& index_buffer)
        {
            _index_buffer = std::move(index_buffer);
        }

        void render()
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

        VertexBuffer& get_vertex_buffer() { return _vertex_buffer; }
        IndexBuffer& get_index_buffer() { return _index_buffer; }

    private:
        uint32_t _id = 0;
        uint32_t _mode = GL_POINTS;

        VertexBuffer _vertex_buffer;
        IndexBuffer _index_buffer;
    };
}

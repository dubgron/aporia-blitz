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
            this->_id   = other._id;
            this->_mode = other._mode;
            this->_vbo  = std::move(other._vbo);
            this->_ibo  = std::move(other._ibo);

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

        void set_vertex_buffer(VertexBuffer&& vbo)
        {
            _vbo = std::move(vbo);
        }

        void set_index_buffer(IndexBuffer&& ibo)
        {
            _ibo = std::move(ibo);
        }

        void render()
        {
            _vbo.flush();

            this->bind();
            _ibo.bind();

            const uint32_t count = static_cast<uint32_t>(_vbo.size() / _vbo.vertex_count() * _ibo.index_count());
            glDrawElements(_mode, count, GL_UNSIGNED_INT, nullptr);

            _ibo.unbind();
            this->unbind();

            _vbo.clear();
        }

        VertexBuffer& get_vertex_buffer() { return _vbo; }
        IndexBuffer& get_index_buffer() { return _ibo; }

    private:
        uint32_t _id = 0;
        uint32_t _mode = GL_POINTS;

        VertexBuffer _vbo;
        IndexBuffer _ibo;
    };
}

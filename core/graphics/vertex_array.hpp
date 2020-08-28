#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include <GL/gl3w.h>

#include "graphics/index_buffer.hpp"
#include "graphics/vertex_buffer.hpp"

namespace Aporia
{
    template<size_t Size, size_t VertexCount, size_t IndexCount>
    class VertexArray final
    {
        using VertexBuffer = VertexBuffer<Size, VertexCount>;
        using IndexBuffer = IndexBuffer<Size, IndexCount>;

    public:
        VertexArray()
        {
            glGenVertexArrays(1, &_id);

            if (VertexCount == 4 && IndexCount == 6)
                _mode = GL_TRIANGLES;
            else if (VertexCount == 2 && IndexCount == 2)
                _mode = GL_LINES;
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

        void set_vertex_buffer(std::shared_ptr<VertexBuffer> vbo)
        {
            _vbo = std::move(vbo);
        }

        void set_index_buffer(std::shared_ptr<IndexBuffer> ibo)
        {
            _ibo = std::move(ibo);
        }

        void render()
        {
            _vbo->flush();

            this->bind();
            _ibo->bind();

            glDrawElements(_mode, get_index_count(), GL_UNSIGNED_INT, nullptr);

            _ibo->unbind();
            this->unbind();

            _vbo->clear();
        }

        size_t get_index_count() const
        {
            return _vbo->size() / VertexCount * IndexCount;
        }

        std::shared_ptr<VertexBuffer> get_vertex_buffer() const { return _vbo; }
        std::shared_ptr<IndexBuffer> get_index_buffer() const { return _ibo; }

    private:
        uint32_t _id = 0;
        uint32_t _mode = GL_POINTS;

        std::shared_ptr<VertexBuffer> _vbo;
        std::shared_ptr<IndexBuffer> _ibo;
    };
}

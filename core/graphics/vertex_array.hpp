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
    template<size_t Size, size_t VertexCount, size_t IndexCount>
    class VertexArray final
    {
        using VertexBufferT = VertexBuffer<Size, VertexCount>;
        using IndexBufferT = IndexBuffer<Size, IndexCount>;

    public:
        VertexArray()
        {
            glGenVertexArrays(1, &_id);

            if constexpr (VertexCount == 4 && IndexCount == 6)
            {
                _mode = GL_TRIANGLES;
            }
            else if constexpr (VertexCount == 2 && IndexCount == 2)
            {
                _mode = GL_LINES;
            }
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

        void set_vertex_buffer(std::shared_ptr<VertexBufferT> vbo)
        {
            _vbo = std::move(vbo);
        }

        void set_index_buffer(std::shared_ptr<IndexBufferT> ibo)
        {
            _ibo = std::move(ibo);
        }

        void render()
        {
            _vbo->flush();

            this->bind();
            _ibo->bind();

            const uint32_t count = static_cast<uint32_t>(_vbo->size() / VertexCount * IndexCount);
            glDrawElements(_mode, count, GL_UNSIGNED_INT, nullptr);

            _ibo->unbind();
            this->unbind();

            _vbo->clear();
        }

        std::shared_ptr<VertexBufferT> get_vertex_buffer() const { return _vbo; }
        std::shared_ptr<IndexBufferT> get_index_buffer() const { return _ibo; }

    private:
        uint32_t _id = 0;
        uint32_t _mode = GL_POINTS;

        std::shared_ptr<VertexBufferT> _vbo;
        std::shared_ptr<IndexBufferT> _ibo;
    };
}

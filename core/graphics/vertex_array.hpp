#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "graphics/index_buffer.hpp"
#include "graphics/vertex_buffer.hpp"

namespace Aporia
{
    class VertexArray final
    {
    public:
        VertexArray(size_t vertex_count, size_t index_count);

        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;

        VertexArray(VertexArray&&) = default;
        VertexArray& operator=(VertexArray&& other) noexcept;

        ~VertexArray();

        void bind() const;
        void unbind() const;

        void set_vertex_buffer(VertexBuffer&& vertex_buffer);
        void set_index_buffer(IndexBuffer&& index_buffer);

        void render();

        VertexBuffer& get_vertex_buffer() { return _vertex_buffer; }
        IndexBuffer& get_index_buffer() { return _index_buffer; }

    private:
        uint32_t _id = 0;
        uint32_t _mode = 0;

        VertexBuffer _vertex_buffer;
        IndexBuffer _index_buffer;
    };
}

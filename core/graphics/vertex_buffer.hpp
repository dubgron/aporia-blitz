#pragma once

#include <algorithm>
#include <utility>
#include <vector>

#include "aporia_types.hpp"
#include "graphics/vertex.hpp"

namespace Aporia
{
    class VertexBuffer final
    {
    public:
        VertexBuffer() = default;
        VertexBuffer(u64 max_objects, u64 count);

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        VertexBuffer(VertexBuffer&&) = default;
        VertexBuffer& operator=(VertexBuffer&& other) noexcept;

        ~VertexBuffer();

        void bind() const;
        void unbind() const;

        void add_layout();

        void flush() const;

        void push(Vertex vertex);
        void emplace(Vertex&& vertex);
        void clear();

        std::vector<Vertex>::iterator begin()                         { return _buffer.begin(); }
        std::vector<Vertex>::iterator end()                           { return _buffer.end(); }
        std::vector<Vertex>::reverse_iterator rbegin()                { return _buffer.rbegin(); }
        std::vector<Vertex>::reverse_iterator rend()                  { return _buffer.rend(); }

        std::vector<Vertex>::const_iterator begin() const             { return _buffer.begin(); }
        std::vector<Vertex>::const_iterator end() const               { return _buffer.end(); }
        std::vector<Vertex>::const_reverse_iterator rbegin() const    { return _buffer.rbegin(); }
        std::vector<Vertex>::const_reverse_iterator rend() const      { return _buffer.rend(); }

        u64 size() const { return _buffer.size(); }
        u64 count() const { return _count; }

    private:
        u32 _id = 0;
        u64 _count = 0;

        std::vector<Vertex> _buffer;
    };
}

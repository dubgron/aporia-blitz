#pragma once

#include <algorithm>
#include <utility>
#include <vector>

#include "graphics/vertex.hpp"

namespace Aporia
{
    class VertexBuffer final
    {
    public:
        VertexBuffer() = default;
        VertexBuffer(size_t max_objects, size_t count);

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        VertexBuffer(VertexBuffer&&) = default;
        VertexBuffer& operator=(VertexBuffer&& other) noexcept;

        ~VertexBuffer();

        void bind() const;
        void unbind() const;

        void add_layout();

        void flush();

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

        size_t size() const { return _buffer.size(); }
        size_t count() const { return _count; }

    private:
        uint32_t _id = 0;
        size_t _count = 0;

        std::vector<Vertex> _buffer;
    };
}

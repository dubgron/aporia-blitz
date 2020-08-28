#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <GL/gl3w.h>

#include "graphics/vertex.hpp"

namespace Aporia
{
    template<size_t Size, size_t Count>
    class VertexBuffer final
    {
    public:
        VertexBuffer()
        {
            glGenBuffers(1, &_id);
            glBindBuffer(GL_ARRAY_BUFFER, _id);

            glNamedBufferData(_id, Size * Count * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

            _buffer.reserve(Size * Count);
        }

        ~VertexBuffer()
        {
            glDeleteBuffers(1, &_id);
        }

        void bind() const
        {
            glBindBuffer(GL_ARRAY_BUFFER, _id);
        }

        void unbind() const
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        template<typename T>
        void add_layout()
        {
            static_assert(false);
        }

        template<>
        void add_layout<Vertex>()
        {
            this->bind();

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Vertex::position));

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)offsetof(Vertex, Vertex::color));

            glEnableVertexAttribArray(2);
            glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (const void*)offsetof(Vertex, Vertex::tex_id));

            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Vertex::tex_coord));

            this->unbind();
        }

        void flush()
        {
            if (!_buffer.empty())
                glNamedBufferSubData(_id, 0, _buffer.size() * sizeof(Vertex), &_buffer[0]);
        }

        void push(Vertex vertex)
        {
            _buffer.push_back(std::move(vertex));
        }

        void clear()
        {
            _buffer.clear();
        }

        std::vector<Vertex>::iterator begin()                         { return _buffer.begin(); }
        std::vector<Vertex>::iterator end()                           { return _buffer.end(); }
        std::vector<Vertex>::reverse_iterator rbegin()                { return _buffer.rbegin(); }
        std::vector<Vertex>::reverse_iterator rend()                  { return _buffer.rend(); }

        std::vector<Vertex>::const_iterator begin() const             { return _buffer.begin(); }
        std::vector<Vertex>::const_iterator end() const               { return _buffer.end(); }
        std::vector<Vertex>::const_reverse_iterator rbegin() const    { return _buffer.rbegin(); }
        std::vector<Vertex>::const_reverse_iterator rend() const      { return _buffer.rend(); }

        size_t size() const { return _buffer.size(); }

    private:
        uint32_t _id = 0;

        std::vector<Vertex> _buffer;
    };
}

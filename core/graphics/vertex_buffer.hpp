#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "graphics/opengl.hpp"
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

#           if defined(APORIA_EMSCRIPTEN)
                glBufferData(GL_ARRAY_BUFFER, Size * Count * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
#           else
                glNamedBufferData(_id, Size * Count * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
#           endif

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

        void add_layout()
        {
            this->bind();

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)offsetof(Vertex, color));

            glEnableVertexAttribArray(2);
#           if defined(APORIA_EMSCRIPTEN)
                glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, tex_id));
#           else
                glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (const void*)offsetof(Vertex, tex_id));
#           endif

            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, tex_coord));

            this->unbind();
        }

        void flush()
        {
            if (!_buffer.empty())
            {
#               if defined(APORIA_EMSCRIPTEN)
                    bind();
                    glBufferSubData(GL_ARRAY_BUFFER, 0, _buffer.size() * sizeof(Vertex), &_buffer[0]);
                    unbind();
#               else
                    glNamedBufferSubData(_id, 0, _buffer.size() * sizeof(Vertex), &_buffer[0]);
#               endif
            }
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

#include "vertex_buffer.hpp"

#include "platform/opengl.hpp"

namespace Aporia
{
    VertexBuffer::VertexBuffer(size_t max_objects, size_t count)
        : _count(count)
    {
        glGenBuffers(1, &_id);
        glBindBuffer(GL_ARRAY_BUFFER, _id);

#       if defined(APORIA_EMSCRIPTEN)
            glBufferData(GL_ARRAY_BUFFER, Size * Count * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
#       else
            glNamedBufferData(_id, max_objects * count * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
#       endif

        _buffer.reserve(max_objects * count);
    }

    VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
    {
        _id = other._id;
        _buffer = std::move(other._buffer);
        _count = other._count;

        other._id = 0;

        return *this;
    }

    VertexBuffer::~VertexBuffer()
    {
        glDeleteBuffers(1, &_id);
    }

    void VertexBuffer::bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, _id);
    }

    void VertexBuffer::unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VertexBuffer::add_layout()
    {
        bind();

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)offsetof(Vertex, color));

#       if defined(APORIA_EMSCRIPTEN)
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, tex_id));
#       else
            glEnableVertexAttribArray(2);
            glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (const void*)offsetof(Vertex, tex_id));
#       endif

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, tex_coord));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, additional));

        unbind();
    }

    void VertexBuffer::flush()
    {
        if (!_buffer.empty())
        {
#           if defined(APORIA_EMSCRIPTEN)
                bind();
                glBufferSubData(GL_ARRAY_BUFFER, 0, _buffer.size() * sizeof(Vertex), &_buffer[0]);
                unbind();
#           else
                glNamedBufferSubData(_id, 0, _buffer.size() * sizeof(Vertex), &_buffer[0]);
#           endif
        }
    }

    void VertexBuffer::push(Vertex vertex)
    {
        _buffer.push_back( std::move(vertex) );
    }

    void VertexBuffer::emplace(Vertex&& vertex)
    {
        _buffer.push_back( std::move(vertex) );
    }

    void VertexBuffer::clear()
    {
        _buffer.clear();
    }
}

#include "index_buffer.hpp"

namespace Aporia
{
    IndexBuffer::IndexBuffer(size_t max_objects, size_t index_count, const std::vector<uint32_t>& indecies)
        : _count(index_count)
    {
        glGenBuffers(1, &_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id);

#       if defined(APORIA_EMSCRIPTEN)
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * count * sizeof(uint32_t), &indecies[0], GL_STATIC_DRAW);
#       else
            glNamedBufferData(_id, max_objects * index_count * sizeof(uint32_t), &indecies[0], GL_STATIC_DRAW);
#       endif
    }

    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
    {
        _id = other._id;
        _count = other._count;

        other._id = 0;

        return *this;
    }

    IndexBuffer::~IndexBuffer()
    {
        glDeleteBuffers(1, &_id);
    }

    void IndexBuffer::bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id);
    }

    void IndexBuffer::unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

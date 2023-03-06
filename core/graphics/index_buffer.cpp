#include "index_buffer.hpp"

namespace Aporia
{
    IndexBuffer::IndexBuffer(u64 max_objects, u64 index_count, const std::vector<u32>& indices)
        : _count(index_count)
    {
        glGenBuffers(1, &_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id);

#if defined(APORIA_EMSCRIPTEN)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_objects * index_count * sizeof(u32), &indecies[0], GL_STATIC_DRAW);
#else
        glNamedBufferData(_id, index_count * max_objects * sizeof(u32), &indices[0], GL_STATIC_DRAW);
#endif
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

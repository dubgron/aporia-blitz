#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <GL/gl3w.h>

namespace Aporia
{
    template<size_t Size, size_t Count>
    class IndexBuffer final
    {
    public:
        IndexBuffer(const std::vector<uint32_t>& indecies)
        {
            glGenBuffers(1, &_id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id);

            glNamedBufferData(_id, Size * Count * sizeof(uint32_t), &indecies[0], GL_STATIC_DRAW);
        }

        ~IndexBuffer()
        {
            glDeleteBuffers(1, &_id);
        }

        void bind() const
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id);
        }

        void unbind() const
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

    private:
        uint32_t _id = 0;
    };
}

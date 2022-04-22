#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "graphics/opengl.hpp"

namespace Aporia
{
    class IndexBuffer final
    {
    public:
        IndexBuffer() = default;

        IndexBuffer(size_t max_objects, size_t index_count, const std::vector<uint32_t>& indecies)
            : _index_count(index_count)
        {
            glGenBuffers(1, &_id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id);

#           if defined(APORIA_EMSCRIPTEN)
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * count * sizeof(uint32_t), &indecies[0], GL_STATIC_DRAW);
#           else
                glNamedBufferData(_id, max_objects * index_count * sizeof(uint32_t), &indecies[0], GL_STATIC_DRAW);
#           endif
        }

        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        IndexBuffer(IndexBuffer&&) = default;
        IndexBuffer& operator=(IndexBuffer&& other) noexcept
        {
            this->_id           = other._id;
            this->_index_count  = other._index_count;

            other._id = 0;

            return *this;
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

        size_t index_count() const { return _index_count; }

    private:
        uint32_t _id = 0;

        size_t _index_count = 0;
    };
}

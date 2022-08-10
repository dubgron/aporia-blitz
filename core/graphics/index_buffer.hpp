#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "platform/opengl.hpp"

namespace Aporia
{
    class IndexBuffer final
    {
    public:
        IndexBuffer() = default;
        IndexBuffer(size_t max_objects, size_t index_count, const std::vector<uint32_t>& indecies);

        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        IndexBuffer(IndexBuffer&&) = default;
        IndexBuffer& operator=(IndexBuffer&& other) noexcept;

        ~IndexBuffer();

        void bind() const;
        void unbind() const;

        size_t count() const { return _count; }

    private:
        uint32_t _id = 0;
        size_t _count = 0;
    };
}

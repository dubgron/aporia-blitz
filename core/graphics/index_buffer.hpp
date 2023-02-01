#pragma once

#include <vector>

#include "aporia_types.hpp"
#include "platform/opengl.hpp"

namespace Aporia
{
    class IndexBuffer final
    {
    public:
        IndexBuffer() = default;
        IndexBuffer(u64 max_objects, u64 index_count, const std::vector<u32>& indices);

        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        IndexBuffer(IndexBuffer&&) = default;
        IndexBuffer& operator=(IndexBuffer&& other) noexcept;

        ~IndexBuffer();

        void bind() const;
        void unbind() const;

        u64 count() const { return _count; }

    private:
        u32 _id = 0;
        u64 _count = 0;
    };
}

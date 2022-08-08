#pragma once

#include <array>
#include <cstdint>
#include <map>

#include "shader.hpp"
#include "vertex.hpp"

namespace Aporia
{
    /* Index for VertexArray in Renderer */
    enum class BufferType : uint8_t
    {
        Quads = 0,
        Lines = 1
    };

    struct RenderQueueKey final
    {
        BufferType buffer = BufferType::Quads;
        ShaderRef program_id = 0;

        std::array<Vertex, 4> vertex;
    };

    static bool operator<(const RenderQueueKey& key1, const RenderQueueKey& key2) noexcept
    {
        return key1.vertex[0].position.z < key2.vertex[0].position.z ||
            (key1.vertex[0].position.z == key2.vertex[0].position.z && key1.buffer < key2.buffer) ||
            (key1.vertex[0].position.z == key2.vertex[0].position.z && key1.buffer == key2.buffer && key1.program_id < key2.program_id);
    }

    using RenderQueue = std::vector<RenderQueueKey>;
}

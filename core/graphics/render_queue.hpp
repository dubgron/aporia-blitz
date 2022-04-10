#pragma once

#include <cstdint>
#include <map>

#include "shader.hpp"
#include "vertex.hpp"

namespace Aporia
{
    enum class BufferType : uint8_t
    {
        None,
        Quads,
        Lines
    };

    struct RenderQueueKey final
    {
        float depth = 0.0f;
        ShaderRef program_id = 0;
        BufferType buffer = BufferType::None;
        bool transparent = false;
    };

    static bool operator<(const RenderQueueKey& key1, const RenderQueueKey& key2) noexcept
    {
        return key1.transparent < key2.transparent ||
            (key1.transparent == key2.transparent && key1.depth < key2.depth) ||
            (key1.transparent == key2.transparent && key1.depth == key2.depth && key1.buffer < key2.buffer) ||
            (key1.transparent == key2.transparent && key1.depth == key2.depth && key1.buffer == key2.buffer && key1.program_id < key2.program_id);
    }

    using RenderQueue = std::multimap<RenderQueueKey, Vertex>;
}

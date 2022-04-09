#pragma once

#include <cstdint>

namespace Aporia
{
    using Shader = const uint32_t;
    using ShaderRef = uint32_t;

    enum class ShaderType : uint8_t
    {
        None,
        Fragment,
        Vertex
    };
}

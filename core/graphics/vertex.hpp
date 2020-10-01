#pragma once 

#include <cstdint>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "components/color.hpp"

namespace Aporia
{
    struct Vertex final
    {
        glm::vec3 position{ 0.0f };
        Color color = Colors::White;
        uint32_t tex_id = 0u;
        glm::vec2 tex_coord{ 0.0f };
    };
}

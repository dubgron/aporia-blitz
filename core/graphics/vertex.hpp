#pragma once 

#include <cstdint>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "components/color.hpp"
#include "graphics/common.hpp"

namespace Aporia
{
    struct Vertex final
    {
        glm::vec3 position{ 0.0f };
        Color color = Colors::White;
        texture_id tex_id = 0;
        glm::vec2 tex_coord{ 0.0f };
    };
}

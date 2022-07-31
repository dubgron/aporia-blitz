#pragma once

#include <utility>

#include <glm/vec3.hpp>

#include "entity.hpp"
#include "components/color.hpp"
#include "components/linear2d.hpp"
#include "components/transform2d.hpp"
#include "graphics/shader.hpp"

namespace Aporia
{
    struct Line2D final : public Entity<Transform2D, Linear2D, Color, ShaderRef>
    {
        Line2D() = default;
        Line2D(glm::vec3 p_1, glm::vec2 p_2, Color color = Colors::Black, ShaderRef shader = 0u);
    };
}

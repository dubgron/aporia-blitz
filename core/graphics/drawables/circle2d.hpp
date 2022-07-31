#pragma once

#include <glm/vec3.hpp>

#include "entity.hpp"
#include "components/circular.hpp"
#include "components/color.hpp"
#include "components/transform2d.hpp"
#include "graphics/shader.hpp"

namespace Aporia
{
    struct Circle2D final : public Entity<Transform2D, Circular, Color, ShaderRef>
    {
        Circle2D() = default;
        Circle2D(float x, float y, float radius, Color color = Colors::Black, ShaderRef shader = 0u);
        Circle2D(glm::vec3 position, float radius, Color color = Colors::Black, ShaderRef shader = 0u);
    };
}

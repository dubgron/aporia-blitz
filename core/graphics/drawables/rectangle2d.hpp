#pragma once

#include <glm/vec3.hpp>

#include "entity.hpp"
#include "components/color.hpp"
#include "components/rectangular.hpp"
#include "components/transform2d.hpp"
#include "graphics/shader.hpp"

namespace Aporia
{
    struct Rectangle2D final : public Entity<Transform2D, Rectangular, Color, ShaderRef>
    {
        Rectangle2D() = default;
        Rectangle2D(float x, float y, float width, float height, Color color = Colors::White, ShaderRef shader = 0);
        Rectangle2D(glm::vec3 position, float width, float height, Color color = Colors::White, ShaderRef shader = 0);
    };
}

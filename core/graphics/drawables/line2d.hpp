#pragma once

#include <glm/vec2.hpp>

#include "components/color.hpp"
#include "components/transform2d.hpp"
#include "graphics/shader.hpp"

namespace Aporia
{
    struct Line2D final
    {
        Line2D();

        Transform2D transform;
        glm::vec2 offset{ 0.0f };

        Color color = Colors::White;
        ShaderRef shader = 0;
    };
}

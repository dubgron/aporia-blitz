#pragma once

#include <glm/vec2.hpp>

#include "aporia_shaders.hpp"
#include "aporia_types.hpp"
#include "components/color.hpp"
#include "components/transform2d.hpp"

namespace Aporia
{
    struct Line2D final
    {
        Line2D();

        Transform2D transform;
        v2 offset{ 0.f };

        Color color = Colors::White;
        ShaderID shader = 0;
    };
}

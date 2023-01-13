#pragma once

#include "components/color.hpp"
#include "components/transform2d.hpp"
#include "graphics/shader.hpp"

namespace Aporia
{
    struct Circle2D final
    {
        Circle2D();

        Transform2D transform;
        float radius = 0.0f;
        Color color = Colors::White;
        ShaderRef shader = 0;
    };
}

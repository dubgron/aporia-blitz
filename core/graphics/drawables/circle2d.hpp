#pragma once

#include "aporia_shaders.hpp"
#include "aporia_types.hpp"
#include "components/color.hpp"
#include "components/transform2d.hpp"

namespace Aporia
{
    struct Circle2D final
    {
        Circle2D();

        Transform2D transform;
        f32 radius = 0.f;
        Color color = Colors::White;
        ShaderID shader = 0;
    };
}

#pragma once

#include "aporia_shaders.hpp"
#include "components/color.hpp"
#include "components/rectangular.hpp"
#include "components/transform2d.hpp"

namespace Aporia
{
    struct Rectangle2D final
    {
        Rectangle2D();

        Transform2D transform;
        Rectangular size;
        Color color = Colors::White;
        ShaderID shader = 0;
    };
}

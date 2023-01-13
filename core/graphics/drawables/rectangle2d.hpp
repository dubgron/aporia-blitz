#pragma once

#include "components/color.hpp"
#include "components/rectangular.hpp"
#include "components/transform2d.hpp"
#include "graphics/shader.hpp"

namespace Aporia
{
    struct Rectangle2D final
    {
        Rectangle2D();

        Transform2D transform;
        Rectangular size;
        Color color = Colors::White;
        ShaderRef shader = 0;
    };
}

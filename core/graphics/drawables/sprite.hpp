#pragma once

#include "components/color.hpp"
#include "components/rectangular.hpp"
#include "components/texture.hpp"
#include "components/transform2d.hpp"
#include "graphics/shader.hpp"

namespace Aporia
{
    struct Sprite final
    {
        Sprite();

        Transform2D transform;
        Texture texture;
        Rectangular rect;
        Color color = Colors::White;
        ShaderRef shader = 0;
    };
}

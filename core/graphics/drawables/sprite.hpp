#pragma once

#include "aporia_textures.hpp"
#include "components/color.hpp"
#include "components/rectangular.hpp"
#include "components/transform2d.hpp"
#include "graphics/shader.hpp"

namespace Aporia
{
    struct Sprite final
    {
        Sprite();

        Transform2D transform;
        SubTexture texture;
        Rectangular rect;
        Color color = Colors::White;
        ShaderRef shader = 0;
    };
}

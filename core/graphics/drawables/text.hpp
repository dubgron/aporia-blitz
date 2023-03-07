#pragma once

#include <string>

#include "aporia_shaders.hpp"
#include "components/color.hpp"
#include "components/transform2d.hpp"
#include "graphics/font.hpp"

namespace Aporia
{
    struct Text final
    {
        Text();

        Transform2D transform;
        Color color = Colors::White;
        ShaderID shader = 0;

        std::string caption;
        const Font* font = nullptr;
    };
}

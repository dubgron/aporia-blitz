#pragma once

#include <string>

#include "components/color.hpp"
#include "components/transform2d.hpp"
#include "graphics/font.hpp"
#include "graphics/shader.hpp"

namespace Aporia
{
    struct Text final
    {
        Text();

        Transform2D transform;
        Color color = Colors::White;
        ShaderRef shader = 0;

        std::string caption;
        const Font* font = nullptr;
    };
}

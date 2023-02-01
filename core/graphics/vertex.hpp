#pragma once

#include "aporia_types.hpp"
#include "components/color.hpp"
#include "platform/opengl.hpp"

namespace Aporia
{
    struct Vertex final
    {
        v3 position{ 0.f };

        Color color = Colors::White;

        texture_id tex_id = 0;
        v2 tex_coord{ 0.f };

        f32 additional = 0.f;
    };
}

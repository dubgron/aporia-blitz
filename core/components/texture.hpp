#pragma once

#include "aporia_types.hpp"
#include "platform/opengl.hpp"

namespace Aporia
{
    struct Texture final
    {
        v2 u{ 0.f };
        v2 v{ 0.f };

        struct Origin final
        {
            texture_id id = 0;
            i32 width = 0;
            i32 height = 0;
            i32 channels = 0;
        } origin;
    };
}

#pragma once

#include "aporia_types.hpp"

namespace Aporia
{
    struct Transform2D final
    {
        v3 position{ 0.f };

        v2 origin{ 0.f };
        f32 rotation = 0.f;

        v2 scale{ 1.f };
    };
}

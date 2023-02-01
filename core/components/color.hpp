#pragma once

#include <cstdint>

#include "aporia_types.hpp"

namespace Aporia
{
    struct Color final
    {
        u8 r = 255;
        u8 g = 255;
        u8 b = 255;
        u8 a = 255;
    };

    namespace Colors
    {
        inline constexpr Color Black       = Color{  0,   0,   0,  255 };
        inline constexpr Color White       = Color{ 255, 255, 255, 255 };
        inline constexpr Color Red         = Color{ 255,  0,   0,  255 };
        inline constexpr Color Green       = Color{  0,  255,  0,  255 };
        inline constexpr Color Blue        = Color{  0,   0,  255, 255 };
        inline constexpr Color Yellow      = Color{ 255, 255,  0,  255 };
        inline constexpr Color Magenta     = Color{ 255,  0,  255, 255 };
        inline constexpr Color Cyan        = Color{  0,  255, 255, 255 };
        inline constexpr Color Transparent = Color{  0,   0,   0,   0  };
    }
}

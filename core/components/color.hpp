#pragma once

#include <cstdint>

namespace Aporia
{
    struct Color final
    {
        uint8_t r = 255;
        uint8_t g = 255;
        uint8_t b = 255;
        uint8_t a = 255;
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

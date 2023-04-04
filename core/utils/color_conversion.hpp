#pragma once

#include <cmath>

#include "aporia_types.hpp"
#include "components/color.hpp"

namespace Aporia
{
    inline Color hsv_to_rgb(i32 h, f64 s, f64 v)
    {
        const f64 C = s * v;
        const f64 X = C * (1.0 - std::abs(std::fmod(h / 60.0, 2.0) - 1.0));
        const f64 m = v - C;
        f64 Rs, Gs, Bs;

        if (h >= 0 && h < 60)
        {
            Rs = C;
            Gs = X;
            Bs = 0;
        }
        else if (h >= 60 && h < 120)
        {
            Rs = X;
            Gs = C;
            Bs = 0;
        }
        else if (h >= 120 && h < 180)
        {
            Rs = 0;
            Gs = C;
            Bs = X;
        }
        else if (h >= 180 && h < 240)
        {
            Rs = 0;
            Gs = X;
            Bs = C;
        }
        else if (h >= 240 && h < 300)
        {
            Rs = X;
            Gs = 0;
            Bs = C;
        }
        else
        {
            Rs = C;
            Gs = 0;
            Bs = X;
        }

        const u8 r = static_cast<u8>( (Rs + m) * 255 );
        const u8 g = static_cast<u8>( (Gs + m) * 255 );
        const u8 b = static_cast<u8>( (Bs + m) * 255 );

        return Color{ r, g, b };
    }
}

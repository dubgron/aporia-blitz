#pragma once

#include <cmath>
#include <cstdint>

#include "components/color.hpp"

namespace Aporia
{
    inline Color hsv_to_rgb(int h, double s, double v)
    {
        const double C = s * v;
        const double X = C * (1.0 - std::abs(std::fmod(h / 60.0, 2.0) - 1.0));
        const double m = v - C;
        double Rs, Gs, Bs;

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

        const uint8_t r = static_cast<uint8_t>(Rs + m) * 255;
        const uint8_t g = static_cast<uint8_t>(Gs + m) * 255;
        const uint8_t b = static_cast<uint8_t>(Bs + m) * 255;

        return Color{ r, g, b };
    }
}

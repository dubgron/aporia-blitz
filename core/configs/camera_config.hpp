#pragma once

#include "aporia_types.hpp"
#include "components/color.hpp"
#include "inputs/keyboard.hpp"

namespace Aporia
{
    struct CameraConfig final
    {
        f32 fov = 1.f;
        f32 aspect_ratio = 1.f;
        Color background_color;

        f32 movement_speed = 0.f;
        f32 rotation_speed = 0.f;
        f32 zoom_speed = 0.f;

        Keyboard movement_key_up = Keyboard::W;
        Keyboard movement_key_down = Keyboard::S;
        Keyboard movement_key_left = Keyboard::A;
        Keyboard movement_key_right = Keyboard::D;

        Keyboard rotation_key_left = Keyboard::Q;
        Keyboard rotation_key_right = Keyboard::E;

        Keyboard zoom_key_in = Keyboard::Space;
        Keyboard zoom_key_out = Keyboard::LShift;

        f32 zoom_max = 1.f;
        f32 zoom_min = 1.f;
    };
}

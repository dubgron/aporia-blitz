#pragma once

#include <SFML/Graphics/Color.hpp>

#include "inputs/keyboard.hpp"

namespace Aporia
{
    struct CameraConfig final
    {
        float aspect_ratio = 0.0f;
        float size = 0.0f;
        sf::Color background_color;

        float movement_speed = 0.0f;
        float rotation_speed = 0.0f;
        float zoom_speed = 0.0f;

        Keyboard movement_key_up = Keyboard::W;
        Keyboard movement_key_down = Keyboard::S;
        Keyboard movement_key_left = Keyboard::A;
        Keyboard movement_key_right = Keyboard::D;

        Keyboard rotation_key_left = Keyboard::Q;
        Keyboard rotation_key_right = Keyboard::E;

        Keyboard zoom_key_in = Keyboard::Space;
        Keyboard zoom_key_out = Keyboard::LShift;

        float zoom_max = 0.0f;
        float zoom_min = 0.0f;
    };
}

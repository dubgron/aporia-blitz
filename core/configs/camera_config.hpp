#pragma once

#include <SFML/Graphics/Color.hpp>

#include "inputs/keyboard.hpp"
#include "platform.hpp"

namespace Aporia
{
    struct APORIA_API CameraConfig final
    {
        float aspect_ratio;
        float size;
        sf::Color background_color;

        float movement_speed;
        float rotation_speed;
        float zoom_speed;

        Keyboard movement_key_up;
        Keyboard movement_key_down;
        Keyboard movement_key_left;
        Keyboard movement_key_right;

        Keyboard rotation_key_left;
        Keyboard rotation_key_right;

        Keyboard zoom_key_in;
        Keyboard zoom_key_out;

        float zoom_max;
        float zoom_min;
    };
}

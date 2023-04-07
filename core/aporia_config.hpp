#pragma once

#include <optional>
#include <string>

#include "aporia_entity.hpp"
#include "aporia_inputs.hpp"
#include "aporia_shaders.hpp"

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

    struct ShaderConfig final
    {
        ShaderProperties default_properties;
    };

    struct WindowConfig final
    {
        std::string title;

        u32 width = 0;
        u32 height = 0;

        bool vsync = true;

        std::optional<v2_i32> position = std::nullopt;
    };

    extern WindowConfig window_config;
    extern ShaderConfig shader_config;
    extern CameraConfig camera_config;

    void load_config(std::string_view filepath);
}

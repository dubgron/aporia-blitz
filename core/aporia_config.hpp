#pragma once

#include <optional>
#include <string>

#include "aporia_entity.hpp"
#include "aporia_inputs.hpp"
#include "aporia_shaders.hpp"

namespace Aporia
{
    struct CameraConfig
    {
        f32 fov = 1.f;
        f32 aspect_ratio = 1.f;
        Color background_color;

        f32 movement_speed = 0.f;
        f32 rotation_speed = 0.f;
        f32 zoom_speed = 0.f;

        Key movement_key_up = Key::W;
        Key movement_key_down = Key::S;
        Key movement_key_left = Key::A;
        Key movement_key_right = Key::D;

        Key rotation_key_left = Key::Q;
        Key rotation_key_right = Key::E;

        Key zoom_key_in = Key::Space;
        Key zoom_key_out = Key::LShift;

        f32 zoom_max = 1.f;
        f32 zoom_min = 1.f;
    };

    struct ShaderConfig
    {
        ShaderProperties default_properties;
    };

    struct WindowConfig
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

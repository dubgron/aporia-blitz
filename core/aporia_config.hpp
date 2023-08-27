#pragma once

#include "aporia_entity.hpp"
#include "aporia_inputs.hpp"
#include "aporia_shaders.hpp"
#include "aporia_strings.hpp"

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

    struct EditorConfig
    {
        bool display_editor_grid = false;
    };

    struct ShaderConfig
    {
        ShaderProperties default_properties;
    };

    struct WindowConfig
    {
        String title;

        i32 width = 0;
        i32 height = 0;

        bool vsync = true;

        v2_i32 position = INVALID_POSITION;

        static constexpr v2_i32 INVALID_POSITION{ -1 };
    };

    extern WindowConfig window_config;
    extern ShaderConfig shader_config;
    extern EditorConfig editor_config;
    extern CameraConfig camera_config;

    struct Config_Property
    {
        Config_Property* next = nullptr;
        Config_Property* prev = nullptr;

        String category;
        String field;

        Config_Property* outer = nullptr;
        Config_Property* inner = nullptr;
        StringList literals;
    };

    // @TODO(dubgron): Add better syntax error handling.
    Config_Property* parse_config_from_file(MemoryArena* arena, String filepath);

    bool load_engine_config(String filepath);
}

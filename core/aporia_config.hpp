#pragma once

#include "aporia_assets.hpp"
#include "aporia_input.hpp"
#include "aporia_shaders.hpp"
#include "aporia_string.hpp"
#include "aporia_utils.hpp"

struct CameraConfig
{
    f32 fov = 1.f;
    Color background_color;

    f32 movement_speed = 0.f;
    f32 rotation_speed = 0.f;
    f32 zoom_speed = 0.f;

    Key movement_key_up = Key_W;
    Key movement_key_down = Key_S;
    Key movement_key_left = Key_A;
    Key movement_key_right = Key_D;

    Key rotation_key_left = Key_Q;
    Key rotation_key_right = Key_E;

    Key zoom_key_in = Key_Space;
    Key zoom_key_out = Key_LShift;

    f32 zoom_max = 1.f;
    f32 zoom_min = 1.f;
};

#if defined(APORIA_EDITOR)
struct EditorConfig
{
    bool display_editor_grid = false;
};
#endif

struct ShaderConfig
{
    ShaderProperties default_properties;
};

struct RenderingConfig
{
    i32 custom_resolution_width = 0;
    i32 custom_resolution_height = 0;

    bool is_using_custom_resolution() const
    {
        return custom_resolution_width > 0 && custom_resolution_height > 0;
    }
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
extern RenderingConfig rendering_config;
extern ShaderConfig shader_config;
extern CameraConfig camera_config;

#if defined(APORIA_EDITOR)
extern EditorConfig editor_config;
#endif

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

bool reload_config_asset(Asset* config_asset);
bool load_engine_config(String filepath);

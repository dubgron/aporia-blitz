#include "aporia_config.hpp"

#include "aporia_camera.hpp"
#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_parser.hpp"
#include "aporia_rendering.hpp"
#include "aporia_utils.hpp"
#include "aporia_window.hpp"

WindowConfig window_config;
RenderingConfig rendering_config;
ShaderConfig shader_config;
CameraConfig camera_config;

#if defined(APORIA_EDITOR)
EditorConfig editor_config;
#endif

static void get_value_from_field(ParseTreeNode* node, Key* out_value)
{
    String string;
    get_value_from_field(node, &string);
    *out_value = string_to_key(string);
}

static void get_value_from_field(ParseTreeNode* node, ShaderBlend* out_value, i64 count)
{
    ScratchArena temp = scratch_begin();
    {
        String* strings = arena_push_uninitialized<String>(temp.arena, count);
        get_value_from_field(node, strings, count);

        for (i64 idx = 0; idx < count; ++idx)
            out_value[idx] = string_to_shader_blend(strings[idx]);
    }
    scratch_end(temp);
}

static void get_value_from_field(ParseTreeNode* node, ShaderBlendOp* out_value)
{
    String string;
    get_value_from_field(node, &string);
    *out_value = string_to_shader_blend_op(string);
}

static void get_value_from_field(ParseTreeNode* node, ShaderDepthTest* out_value)
{
    String string;
    get_value_from_field(node, &string);
    *out_value = string_to_shader_depth_test(string);
}

static void get_value_from_field(ParseTreeNode* node, ShaderDepthWrite* out_value)
{
    String string;
    get_value_from_field(node, &string);
    *out_value = string_to_shader_depth_write(string);
}

static bool load_engine_config_from_file(String filepath)
{
    ScratchArena temp = scratch_begin();
    ParseTreeNode* parsed_config = parse_from_file(temp.arena, filepath);

    if (!parsed_config)
    {
        scratch_end(temp);
        return false;
    }

    for (ParseTreeNode* node = parsed_config->child_first; node; node = node->next)
    {
        APORIA_ASSERT(node->type == ParseTreeNode_Category);

        if (node->name == "window")
        {
            for (ParseTreeNode* window_node = node->child_first; window_node; window_node = window_node->next)
            {
                APORIA_ASSERT(window_node->type == ParseTreeNode_Field);
                if (window_node->name == "title")
                {
                    get_value_from_field(window_node, &window_config.title);
                    window_config.title = push_string(&memory.persistent, window_config.title);
                }
                else if (window_node->name == "size")
                {
                    get_value_from_field(window_node, &window_config.width, 2);
                }
                else if (window_node->name == "vsync")
                {
                    get_value_from_field(window_node, &window_config.vsync);
                }
                else if (window_node->name == "position")
                {
                    get_value_from_field(window_node, &window_config.position[0], 2);
                }
            }
        }
        else if (node->name == "camera")
        {
            for (ParseTreeNode* camera_node = node->child_first; camera_node; camera_node = camera_node->next)
            {
                APORIA_ASSERT(camera_node->type == ParseTreeNode_Field);
                if (camera_node->name == "fov")
                {
                    get_value_from_field(camera_node, &camera_config.fov);
                }
                else if (camera_node->name == "background_color")
                {
                    get_value_from_field(camera_node, &camera_config.background_color.r, 4);
                }
                else if (camera_node->name == "movement_speed")
                {
                    get_value_from_field(camera_node, &camera_config.movement_speed);
                }
                else if (camera_node->name == "rotation_speed")
                {
                    get_value_from_field(camera_node, &camera_config.rotation_speed);
                }
                else if (camera_node->name == "zoom_speed")
                {
                    get_value_from_field(camera_node, &camera_config.zoom_speed);
                }
                else if (camera_node->name == "zoom_max")
                {
                    get_value_from_field(camera_node, &camera_config.zoom_max);
                }
                else if (camera_node->name == "zoom_min")
                {
                    get_value_from_field(camera_node, &camera_config.zoom_min);
                }
            }
        }
        else if (node->name == "shaders")
        {
            for (ParseTreeNode* shader_node = node->child_first; shader_node; shader_node = shader_node->next)
            {
                if (shader_node->name == "defaults")
                {
                    APORIA_ASSERT(shader_node->type == ParseTreeNode_Struct);
                    for (ParseTreeNode* defaults_node = shader_node->child_first; defaults_node; defaults_node = defaults_node->next)
                    {
                        APORIA_ASSERT(defaults_node->type == ParseTreeNode_Field);
                        if (defaults_node->name == "blend")
                        {
                            get_value_from_field(defaults_node, &shader_config.default_properties.blend[0], 2);
                        }
                        else if (defaults_node->name == "blend_op")
                        {
                            get_value_from_field(defaults_node, &shader_config.default_properties.blend_op);
                        }
                        else if (defaults_node->name == "depth_test")
                        {
                            get_value_from_field(defaults_node, &shader_config.default_properties.depth_test);
                        }
                        else if (defaults_node->name == "depth_write")
                        {
                            get_value_from_field(defaults_node, &shader_config.default_properties.depth_write);
                        }
                    }
                }
            }
        }
        else if (node->name == "rendering")
        {
            for (ParseTreeNode* rendering_node = node->child_first; rendering_node; rendering_node = rendering_node->next)
            {
                APORIA_ASSERT(rendering_node->type == ParseTreeNode_Field);
                if (rendering_node->name == "custom_game_resolution")
                {
                    get_value_from_field(rendering_node, &rendering_config.custom_game_resolution_width, 2);
                }
                else if (rendering_node->name == "custom_ui_resolution")
                {
                    get_value_from_field(rendering_node, &rendering_config.custom_ui_resolution_width, 2);
                }
            }
        }
#if defined(APORIA_EDITOR)
        else if (node->name == "editor")
        {
            for (ParseTreeNode* editor_node = node->child_first; editor_node; editor_node = editor_node->next)
            {
                APORIA_ASSERT(editor_node->type == ParseTreeNode_Field);
                if (editor_node->name == "display_editor_grid")
                {
                    get_value_from_field(editor_node, &editor_config.display_editor_grid);
                }
                else if (editor_node->name == "editor_grid_size")
                {
                    get_value_from_field(editor_node, &editor_config.editor_grid_size);
                }
            }
        }
#endif
    }

    scratch_end(temp);

    return true;
}

#undef PROPERTY_HELPER

bool reload_config_asset(Asset* config_asset)
{
    APORIA_ASSERT(config_asset->type == AssetType::Config);

    arena_clear(&memory.config);

    // Reset configs to perform a clean reload.
    window_config = {};
    rendering_config = {};
    shader_config = {};
    camera_config = {};

#if defined(APORIA_EDITOR)
    editor_config = {};
#endif

    bool success = load_engine_config_from_file(config_asset->source_file);
    config_asset->status = success ? AssetStatus::Loaded : AssetStatus::Unloaded;

    window_apply_config();
    camera_apply_config(&active_camera);

    return success;
}

bool load_engine_config(String filepath)
{
    bool success = load_engine_config_from_file(filepath);

    Asset* config_asset = register_asset(filepath, AssetType::Config);
    config_asset->status = success ? AssetStatus::Loaded : AssetStatus::Unloaded;

    return success;
}

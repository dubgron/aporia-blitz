#include "aporia_config.hpp"

#include <filesystem>

#include <magic_enum.hpp>
#include <nlohmann/json.hpp>

#include "aporia_inputs.hpp"
#include "aporia_shaders.hpp"
#include "common.hpp"
#include "components/color.hpp"
#include "utils/read_file.hpp"

namespace Aporia
{
    using json = nlohmann::json;

    WindowConfig window_config;
    ShaderConfig shader_config;
    CameraConfig camera_config;

    void load_window_config(const json& config)
    {
        const auto& window = config["window_config"];

        window_config.title = window["title"];
        window_config.width = window["width"];
        window_config.height = window["height"];
        window_config.vsync = window["vsync"];

        const auto& position = window.find("position");
        if (position != window.end())
        {
            window_config.position = std::make_optional<v2_i32>(position->at(0), position->at(1));
        }
    }

    void load_shader_config(const json& config)
    {
        const auto& shader = config["shader_config"];
        const auto& default_properties = shader["default_properties"];

        shader_config.default_properties.blend[0] = string_to_shader_blend(default_properties["blend"][0].get<std::string_view>());
        shader_config.default_properties.blend[1] = string_to_shader_blend(default_properties["blend"][1].get<std::string_view>());
        shader_config.default_properties.blend_op = string_to_shader_blend_op(default_properties["blend_op"].get<std::string_view>());
        shader_config.default_properties.depth_test = string_to_shader_depth_test(default_properties["depth_test"].get<std::string_view>());
        shader_config.default_properties.depth_write = string_to_shader_depth_write(default_properties["depth_write"].get<std::string_view>());
    }

    void load_camera_config(const json& config)
    {
        const auto& camera = config["camera_config"];
        const auto& bg_color = camera["background_color"];

        camera_config.fov = camera["fov"];
        camera_config.aspect_ratio = camera["aspect_ratio"];
        camera_config.background_color = Color{ bg_color[0], bg_color[1], bg_color[2], bg_color[3] };
        camera_config.movement_speed = camera["movement_speed"];
        camera_config.rotation_speed = camera["rotation_speed"];
        camera_config.zoom_speed = camera["zoom_speed"];

        auto get_key = [&camera](const char* key)
            {
                return magic_enum::enum_cast<Keyboard>(camera[key].get<std::string_view>()).value();
            };

        camera_config.movement_key_up = get_key("movement_key_up");
        camera_config.movement_key_down = get_key("movement_key_down");
        camera_config.movement_key_left = get_key("movement_key_left");
        camera_config.movement_key_right = get_key("movement_key_right");
        camera_config.zoom_key_in = get_key("zoom_key_in");
        camera_config.zoom_key_out = get_key("zoom_key_out");

        camera_config.zoom_max = camera["zoom_max"];
        camera_config.zoom_min = camera["zoom_min"];
    }

    void load_config(std::string_view filepath)
    {
        APORIA_ASSERT_WITH_MESSAGE(std::filesystem::exists(filepath),
            "Config file '{}' doesn't exist!", filepath);

        // @TODO(dubgron): Handling when json file is not correct.
        std::string json_contents = read_file(filepath);
        const json config_json = json::parse( std::move(json_contents) );

        load_window_config(config_json);
        load_shader_config(config_json);
        load_camera_config(config_json);
    }
}

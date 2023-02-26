#include "config_manager.hpp"

#include <filesystem>
#include <string_view>
#include <vector>

#include <glm/vec2.hpp>
#include <magic_enum.hpp>
#include <nlohmann/json.hpp>

#include "common.hpp"
#include "event_manager.hpp"
#include "components/color.hpp"
#include "inputs/keyboard.hpp"
#include "utils/read_file.hpp"

namespace Aporia
{
    ConfigManager::ConfigManager(EventManager& events, const std::string& path)
        : _events(events), _path(path)
    {
        if (!std::filesystem::exists(_path))
        {
            APORIA_LOG(Critical, "Config file '{}' doesn't exist!", _path);
            return;
        }

        /* TODO: Handling when json file is not correct */
        using json = nlohmann::json;
        const json config_json = load_json(_path);

        load_window_config(config_json);
        load_texture_config(config_json);
        load_shader_config(config_json);
        load_camera_config(config_json);
    }

    void ConfigManager::reload()
    {
        using json = nlohmann::json;
        json config_json = load_json(_path);

        load_window_config(config_json);
        _events.call_event<ReloadWindowConfigEvent>();

        load_texture_config(config_json);
        _events.call_event<ReloadTextureConfigEvent>();

        load_shader_config(config_json);
        _events.call_event<ReloadShaderConfigEvent>();

        load_camera_config(config_json);
        _events.call_event<ReloadCameraConfigEvent>();
    }

    void ConfigManager::load_window_config(const json& config)
    {
        const auto& window = config["window_config"];

        window_config.title = window["title"];
        window_config.width = window["width"];
        window_config.height = window["height"];
        window_config.vsync = window["vsync"];

        const auto& position = window.find("position");
        window_config.position = position != window.end()
            ? std::make_optional<glm::ivec2>(position->at(0), position->at(1))
            : std::nullopt;
    }

    void ConfigManager::load_texture_config(const json& config)
    {
        texture_config.atlas = config["texture_config"];
    }

    void ConfigManager::load_shader_config(const json& config)
    {
        const auto& shader = config["shader_config"];
        const auto& default_properties = shader["default_properties"];

        shader_config.default_properties.blend[0] = string_to<ShaderBlend>(default_properties["blend"][0].get<std::string_view>());
        shader_config.default_properties.blend[1] = string_to<ShaderBlend>(default_properties["blend"][1].get<std::string_view>());
        shader_config.default_properties.blend_op = string_to<ShaderBlendOp>(default_properties["blend_op"].get<std::string_view>());
        shader_config.default_properties.depth_test = string_to<ShaderDepthTest>(default_properties["depth_test"].get<std::string_view>());
        shader_config.default_properties.depth_write = string_to<ShaderDepthWrite>(default_properties["depth_write"].get<std::string_view>());
    }

    void ConfigManager::load_camera_config(const json& config)
    {
        const auto& camera = config["camera_config"];
        const auto& bg_color = camera["background_color"];

        camera_config.fov = camera["fov"];
        camera_config.aspect_ratio = camera["aspect_ratio"];
        camera_config.background_color = Color{ bg_color[0], bg_color[1], bg_color[2], bg_color[3] };
        camera_config.movement_speed = camera["movement_speed"];
        camera_config.rotation_speed = camera["rotation_speed"];
        camera_config.zoom_speed = camera["zoom_speed"];

        auto get_key = [&camera](const char* key) { return magic_enum::enum_cast<Keyboard>(camera[key].get<std::string_view>()).value(); };

        camera_config.movement_key_up = get_key("movement_key_up");
        camera_config.movement_key_down = get_key("movement_key_down");
        camera_config.movement_key_left = get_key("movement_key_left");
        camera_config.movement_key_right = get_key("movement_key_right");
        camera_config.zoom_key_in = get_key("zoom_key_in");
        camera_config.zoom_key_out = get_key("zoom_key_out");

        camera_config.zoom_max = camera["zoom_max"];
        camera_config.zoom_min = camera["zoom_min"];
    }
}

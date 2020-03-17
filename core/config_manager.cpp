#include "config_manager.hpp"

#include <filesystem>

#include <nlohmann/json.hpp>

#include "utils/read_file.hpp"

namespace Aporia
{
    ConfigManager::ConfigManager(Logger& logger, const std::string& config)
        : _logger(logger)
    {
        if (!std::filesystem::exists(config))
        {
            _logger.log(LOG_CRITICAL) << "Config file '" << config << "' doesn't exist!";
            return;
        }

        using json = nlohmann::json;
        
        std::string data = read_file(config);
        json config_json = json::parse(data);

        /* TODO: Handling when json file is not correct */

        /* Getting Window Config */
        auto window = config_json["window_config"];

        window_config.title = window["title"];
        window_config.position = { window["position"][0], window["position"][1] };
        window_config.width = window["width"];
        window_config.height = window["height"];
        window_config.framerate = window["framerate"];

        /* Getting Texture Config */
        const auto& texture = config_json["texture_config"];

        texture_config.atlas = texture["src"];

        /* Getting Camera Config */
        const auto& camera = config_json["camera_config"];
        const auto& bg_color = camera["background_color"];

        camera_config.aspect_ratio = camera["aspect_ratio"];
        camera_config.size = camera["size"];
        camera_config.background_color = sf::Color(bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
        camera_config.movement_speed = camera["movement_speed"];
        camera_config.rotation_speed = camera["rotation_speed"];
        camera_config.zoom_speed = camera["zoom_speed"];
        camera_config.movement_key_up = camera["movement_key_up"];
        camera_config.movement_key_down = camera["movement_key_down"];
        camera_config.movement_key_left = camera["movement_key_left"];
        camera_config.movement_key_right = camera["movement_key_right"];
        camera_config.rotation_key_left = camera["rotation_key_left"];
        camera_config.rotation_key_right = camera["rotation_key_right"];
        camera_config.zoom_key_in = camera["zoom_key_in"];
        camera_config.zoom_key_out = camera["zoom_key_out"];
        camera_config.zoom_max = camera["zoom_max"];
        camera_config.zoom_min = camera["zoom_min"];

        _good = true;
    }

    bool ConfigManager::is_good() const
    {
        return _good;
    }
}

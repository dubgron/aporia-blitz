#include "config_manager.hpp"

#include <filesystem>
#include <string_view>
#include <vector>

#include <glm/vec2.hpp>
#include <magic_enum.hpp>
#include <nlohmann/json.hpp>

#include "components/color.hpp"
#include "inputs/keyboard.hpp"
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
        const auto& window = config_json["window_config"];
        const auto& position = window["position"];

        window_config.title = window["title"];
        window_config.position = glm::ivec2(position[0], position[1]);
        window_config.width = window["width"];
        window_config.height = window["height"];
        window_config.vsync = window["vsync"];

        /* Getting Texture Config */
        const auto& texture = config_json["texture_config"];

        texture_config.atlas = texture["src"];

        /* Getting Camera Config */
        const auto& camera = config_json["camera_config"];
        const auto& bg_color = camera["background_color"];

        camera_config.aspect_ratio = camera["aspect_ratio"];
        camera_config.size = camera["size"];
        camera_config.background_color = Color{ bg_color[0], bg_color[1], bg_color[2], bg_color[3] };
        camera_config.movement_speed = camera["movement_speed"];
        camera_config.rotation_speed = camera["rotation_speed"];
        camera_config.zoom_speed = camera["zoom_speed"];

        auto get_key = [&camera](const char* key){ return magic_enum::enum_cast<Keyboard>(camera[key].get<std::string_view>()).value(); };

        camera_config.movement_key_up = get_key("movement_key_up");
        camera_config.movement_key_down = get_key("movement_key_down");
        camera_config.movement_key_left = get_key("movement_key_left");
        camera_config.movement_key_right = get_key("movement_key_right");
        camera_config.zoom_key_in = get_key("zoom_key_in");
        camera_config.zoom_key_out = get_key("zoom_key_out");

        camera_config.zoom_max = camera["zoom_max"];
        camera_config.zoom_min = camera["zoom_min"];

        /* Getting Animation Config */
        std::string animation_data = read_file(config_json["animation_config"]);
        json animation_json = json::parse(animation_data);

        for (const auto& animation : animation_json["animations"])
        {
            std::vector<AnimationFrameConfig> frames;
            frames.reserve(animation["frames"].size());

            for (const auto& frame : animation["frames"])
                frames.push_back(AnimationFrameConfig{ frame["texture"], frame["duration"] });

            animation_config.animations.try_emplace(animation["name"], std::move(frames));
        }

        _good = true;
    }

    bool ConfigManager::is_good() const
    {
        return _good;
    }
}

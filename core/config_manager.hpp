#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "event_manager.hpp"
#include "logger.hpp"
#include "configs/all_configs.hpp"

namespace Aporia
{
    struct ConfigManager final
    {
        ConfigManager(Logger& logger, EventManager& events, const std::string& path);

        void reload();

        WindowConfig window_config;
        TextureConfig texture_config;
        ShaderConfig shader_config;
        CameraConfig camera_config;
        AnimationConfig animation_config;

    private:
        Logger& _logger;
        EventManager& _events;

        std::string _path = "";

        using json = nlohmann::json;
        void load_window_config(const json& config);
        void load_texture_config(const json& config);
        void load_shader_config(const json& config);
        void load_camera_config(const json& config);
        void load_animation_config(const json& config);
    };
}

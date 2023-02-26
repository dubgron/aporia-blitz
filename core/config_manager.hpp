#pragma once

#include <string>

#include <nlohmann/json_fwd.hpp>

#include "configs/camera_config.hpp"
#include "configs/shader_config.hpp"
#include "configs/texture_config.hpp"
#include "configs/window_config.hpp"

namespace Aporia
{
    class EventManager;

    struct ConfigManager final
    {
        ConfigManager(EventManager& events, const std::string& path);

        void reload();

        WindowConfig window_config;
        TextureConfig texture_config;
        ShaderConfig shader_config;
        CameraConfig camera_config;

    private:
        EventManager& _events;

        std::string _path = "";

        using json = nlohmann::json;
        void load_window_config(const json& config);
        void load_texture_config(const json& config);
        void load_shader_config(const json& config);
        void load_camera_config(const json& config);
    };
}

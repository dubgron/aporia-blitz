#pragma once

#include <string>

#include "logger.hpp"
#include "configs/all_configs.hpp"

namespace Aporia
{
    struct ConfigManager final
    {
        ConfigManager() = default;
        ConfigManager(Logger& logger, const std::string& config);

        bool is_good() const;

        WindowConfig window_config;
        TextureConfig texture_config;
        CameraConfig camera_config;
        AnimationConfig animation_config;

    private:
        bool _good = false;

        Logger& _logger;
    };
}

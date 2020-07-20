#pragma once

#include <memory>
#include <string>

#include "configs/all_configs.hpp"
#include "logger.hpp"
#include "platform.hpp"

namespace Aporia
{
    struct APORIA_API ConfigManager final
    {
        ConfigManager() = default;
        ConfigManager(Logger& logger, const std::string& config);

        bool is_good() const;

        WindowConfig window_config;
        TextureConfig texture_config;
        CameraConfig camera_config;

    private:
        bool _good = false;

        Logger& _logger;
    };
}

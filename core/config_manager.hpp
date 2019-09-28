#pragma once

#include <memory>
#include <string>
#include <vector>

#include "configs.hpp"
#include "logger.hpp"
#include "platform.hpp"

namespace Aporia
{
    struct APORIA_API ConfigManager final
    {
        ConfigManager() = default;
        ConfigManager(std::shared_ptr<Logger> logger, const std::string& config);

        bool is_good() const;

        std::vector<WindowConfig> window_data;
        TextureData texture_data;

    private:
        bool _good = false;

        std::shared_ptr<Logger> _logger;
    };
}

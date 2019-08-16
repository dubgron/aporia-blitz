#pragma once

#include <string>
#include <vector>

#include "configs.hpp"
#include "platform.hpp"

namespace Aporia
{
    struct APORIA_API ConfigManager
    {
        ConfigManager() = default;
        ConfigManager(const std::string& config);

        std::vector<WindowConfig> window_data;
    };
}

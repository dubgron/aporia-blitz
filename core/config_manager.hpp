#pragma once

#include <memory>
#include <string>

#include "configs.hpp"
#include "logger.hpp"
#include "platform.hpp"

namespace Aporia
{
    struct APORIA_API ConfigManager final
    {
        ConfigManager() = default;
        ConfigManager(Logger& logger, const std::string& config);

        bool is_good() const;

        WindowConfig window_data;
        TextureData texture_data;

    private:
        bool _good = false;

        Logger& _logger;
    };
}

#pragma once

#include <string>

#include "config_manager.hpp"
#include "event_manager.hpp"
#include "logger.hpp"
#include "platform.hpp"

namespace Aporia
{
    class APORIA_API Engine final
    {
    public:
        Engine(const std::string& config_file);

    private:
        std::shared_ptr<Logger> _logger;

        std::unique_ptr<ConfigManager> _configs;
        std::unique_ptr<EventManager> _events;
    };
}

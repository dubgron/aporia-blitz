#include "engine.hpp"

namespace Aporia
{
    Engine::Engine(const std::string& config_file)
    {
        _logger = std::make_shared<Logger>("engine");

        _configs = std::make_unique<ConfigManager>(_logger, config_file);
        if (!_configs->is_ok())
            return;

        _events = std::make_unique<EventManager>(_logger);
    }
}

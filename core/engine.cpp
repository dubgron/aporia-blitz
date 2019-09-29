#include "engine.hpp"

namespace Aporia
{
    Engine::Engine(const std::string& config_file)
    {
        _logger = std::make_shared<Logger>("engine");

        _configs = std::make_unique<ConfigManager>(_logger, config_file);
        if (!_configs->is_good())
            return;

        _texture_manager = std::make_unique<TextureManager>(_logger);

        if (!_texture_manager->load_textures(_configs->texture_data))
            return;

        _events = std::make_unique<EventManager>(_logger);

        _renderer = std::make_unique<Renderer>(_logger);
    }
}

#include "engine.hpp"

namespace Aporia
{
    Engine::Engine(const std::string& config_file)
    {
        _logger = std::make_shared<Logger>("engine");

        _configs = std::make_unique<ConfigManager>(_logger, config_file);
        if (!_configs->is_ok())
            return;

		_texture_manager = std::make_unique<TextureManager>(_logger, _configs->texture_data);

		if (_texture_manager->load_textures())
			return;
    }
}

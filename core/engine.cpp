#include "engine.hpp"

namespace Aporia
{
    Engine::Engine(const std::string& config_file)
    {
        _logger = std::make_shared<Logger>("engine");

        _configs = std::make_shared<ConfigManager>(_logger, config_file);
        if (!_configs->is_good())
            return;

        _textures = std::make_shared<TextureManager>(_logger);

        if (!_textures->load_textures(_configs->texture_data))
            return;

        _events = std::make_shared<EventManager>(_logger);
        _inputs = std::make_shared<InputManager>(_logger, _events);

        _renderer = std::make_unique<Renderer>(_logger, _events);
        _window = std::make_shared<Window>(_logger, _configs->window_data);

        _events->add_listener<WindowCloseEvent>([](Window& window) { window.close(); });
    }

    void Engine::run(std::unique_ptr<Game> game)
    {
        game->_configs = _configs;
        game->_events = _events;
        game->_inputs = _inputs;
        game->_textures = _textures;
        game->_logger = std::make_unique<Logger>(_configs->window_data.title);

        game->on_init();

        while (_window->is_open())
        {
            _events->listen_for_events(*_window);
            _window->clear();

            game->on_update();

            _renderer->render(*_window);

            _window->display();
        }

        game->on_terminate();
    }
}

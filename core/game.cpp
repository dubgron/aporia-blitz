#include "game.hpp"

#include <imgui.h>
#include <imgui-SFML.h>

namespace Aporia
{
    Game::Game(const std::string& config_file)
        : _logger("engine"),
          _configs(_logger, config_file),
          _textures(_logger),
          _events(_logger),
          _inputs(_logger, _events),
          _renderer(_logger, _events),
          _window(_logger, _configs.window_config),
          _camera(_logger, _configs.camera_config)
    {
        if (!_configs.is_good())
            return;

        if (!_textures.load_textures(_configs.texture_config))
            return;

        _events.add_listener<WindowCloseEvent>([](Window& window) { window.close(); });
    }

    void Game::run()
    {
        this->on_init();

        sf::Clock time;
        sf::Time delta_time;

        while (_window.is_open())
        {
            delta_time = time.restart();

            _events.listen_for_events(_window);
            _window.clear();

            _window.update(delta_time);
            this->on_update(delta_time);

            _renderer.render(_window, _camera.get_camera());

            _window.display();
        }

        this->on_terminate();
    }
}

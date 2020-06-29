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
          _renderer(_logger),
          _window(_logger, _configs.window_config)
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

        float w = _configs.window_config.width;
        float h = _configs.window_config.height;

        Camera cam(-w / 2 - 1, -w / 2 + 1, -h / 2 - 1, -h / 2 + 1);

        sf::Clock time;
        sf::Time delta_time;
        while (_window.is_open())
        {
            delta_time = time.restart();

            _events.listen_for_events(_window);
            _window.clear();

            _window.update(delta_time);
            this->on_update();

            _renderer.render(_window, cam);

            _window.display();
        }

        this->on_terminate();
    }
}

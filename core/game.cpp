#include "game.hpp"

namespace Aporia
{
    Game::Game(const std::string& config_file)
        : _logger("engine"),
          _configs(_logger, config_file),
          _textures(_logger),
          _events(_logger),
          _inputs(_logger, _events),
          _scenes(_logger),
          _camera(_logger, _events, _configs.camera_config),
          _renderer(_logger),
          _window(_logger, _configs.window_config),
          _layer_stack(_logger),
          _imgui_layer(_logger, _window)
    {
        if (!_configs.is_good())
            return;

        if (!_textures.load_textures(_configs.texture_config))
            return;

        _events.add_listener<WindowCloseEvent>([](Window& window) { window.close(); });

        _layer_stack.push_overlay(_imgui_layer);
    }

    Game::~Game()
    {
        _layer_stack.pop_overlay(_imgui_layer);
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

            _imgui_layer.update(delta_time);
            this->on_update();

            _scenes.get_current_scene()->on_draw(_renderer);

            _renderer.render(_window, _camera.get_camera());

            _imgui_layer.display();
            _window.display();
        }

        this->on_terminate();
    }
}

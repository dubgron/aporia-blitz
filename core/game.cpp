#include "game.hpp"

#include <imgui.h>

namespace Aporia
{
    Game::Game(const std::string& config_file)
        : _logger("engine"),
          _configs(_logger, config_file),
          _events(_logger),
          _window(_logger, _events, _configs.window_config),
          _inputs(_logger, _events),
          _scenes(_logger),
          _textures(_logger, _configs.texture_config),
          _animations(_logger, _textures, _configs.animation_config),
          _camera(_logger, _events, _configs.camera_config),
          _renderer(_logger),
          _layer_stack(_logger),
          _imgui_layer(_logger, _window)
    {
        if (!_configs.is_good())
            return;

        _layer_stack.push_overlay(_imgui_layer);
    }

    Game::~Game()
    {
        _layer_stack.pop_overlay(_imgui_layer);
    }

    void Game::run()
    {
        this->on_init();

        Timer time;
        Deltatime dt;
        while (_window.is_open())
        {
            dt = time.reset();

            _window.poll_events();
            _scenes.get_current_scene()->on_input(_inputs);

            _window.clear(_camera.get_clear_color());

            _imgui_layer.begin();
            _renderer.begin(_camera.get_camera());

            this->on_update(dt);
            _scenes.get_current_scene()->on_update(dt);

            _scenes.get_current_scene()->on_draw(_renderer);

            _renderer.end();
            _imgui_layer.end();

            _window.display();
        }

        this->on_terminate();
    }
}

#include "game.hpp"

#include <imgui.h>

#if defined(APORIA_EMSCRIPTEN)
#   include <emscripten.h>
#endif

namespace Aporia
{
    void main_loop(void* game_ptr)
    {
        reinterpret_cast<Aporia::Game*>(game_ptr)->main_loop();
    }

    Game::Game(const std::string& config_file)
        : _logger("engine"),
          _events(_logger),
          _configs(_logger, _events, config_file),
          _window(_logger, _events, _configs.window_config),
          _inputs(_logger, _events),
          _scenes(_logger),
          _textures(_logger, _events, _configs.texture_config),
          _animations(_logger, _events, _textures, _configs.animation_config),
          _fonts(_logger),
          _shaders(_logger, _configs.shader_config),
          _camera(_logger, _events, _configs.camera_config),
          _renderer(_logger, _shaders, _events, _configs.window_config),
          _layer_stack(_logger),
          _imgui_layer(_logger, _window)
    {
        _layer_stack.push_overlay(_imgui_layer);
    }

    Game::~Game()
    {
        _layer_stack.pop_overlay(_imgui_layer);
    }

    void Game::run()
    {
        this->on_init();

#       if defined(APORIA_EMSCRIPTEN)
            emscripten_set_main_loop_arg(Aporia::main_loop, this, 0, true);
#       else
            while (_window.is_open())
            {
                Aporia::main_loop(this);
            }
#       endif

        this->on_terminate();
    }

    void Game::main_loop()
    {
        _dt = _timer.reset();
        _events.call_event<BeginFrameEvent>();

        _window.poll_events();
        _scenes.get_current_scene()->on_input(_inputs);

        _scenes.get_current_scene()->on_scene_transition(_scenes);

        _imgui_layer.begin();
        _renderer.begin(_camera.get_camera());

        _window.clear(_camera.get_clear_color());

        on_update(_dt);
        _scenes.get_current_scene()->on_update(_dt);

        _scenes.get_current_scene()->on_draw(_renderer);

        _renderer.end();
        _imgui_layer.end();

        _window.display();

        _events.call_event<EndFrameEvent>();
    }
}

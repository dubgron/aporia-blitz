#include "game.hpp"

#include <imgui.h>

#if defined(APORIA_EMSCRIPTEN)
#   include <emscripten.h>
#endif

namespace Aporia
{
    void main_loop(void* game_ptr)
    {
        Aporia::Game* game = (Aporia::Game*)game_ptr;

        game->_dt = game->_timer.reset();
        game->_events.call_event<BeginFrameEvent>();

        game->_window.poll_events();
        game->_scenes.get_current_scene()->on_input(game->_inputs);

        game->_window.clear(game->_camera.get_clear_color());

        game->_scenes.get_current_scene()->on_scene_transition(game->_scenes);

        game->_imgui_layer.begin();
        game->_renderer.begin(game->_camera.get_camera());

        game->on_update(game->_dt);
        game->_scenes.get_current_scene()->on_update(game->_dt);

        game->_scenes.get_current_scene()->on_draw(game->_renderer);

        game->_renderer.end();
        game->_imgui_layer.end();

        game->_window.display();

        game->_events.call_event<EndFrameEvent>();
    }

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
            emscripten_set_main_loop_arg(main_loop, this, 0, true);
#       else
            while (_window.is_open())
                main_loop(this);
#       endif

        this->on_terminate();
    }
}

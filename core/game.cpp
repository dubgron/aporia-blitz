#include "game.hpp"

#if defined(APORIA_EMSCRIPTEN)
    #include <emscripten.h>
#endif

#include "common.hpp"
#include "scene.hpp"

namespace Aporia
{
    void main_loop(void* game_ptr)
    {
        reinterpret_cast<Aporia::Game*>(game_ptr)->main_loop();
    }

    Game::Game(const std::string& config_file)
        : _configs(config_file),
          _inputs(),
          _scenes(),
          _fonts(),
          _shaders(_configs.shader_config),
          _camera(_configs.camera_config),
          _renderer(_shaders),
          _window(_inputs, _renderer, _camera),
          _layer_stack(),
          _imgui_layer(_window)
    {
        _window.init(_configs.window_config);
        _renderer.init(_configs.window_config.width, _configs.window_config.height);
        _world.init();
        _layer_stack.push_overlay(_imgui_layer);
    }

    Game::~Game()
    {
        _layer_stack.pop_overlay(_imgui_layer);
        _world.deinit();
        _renderer.deinit();
        _window.deinit();
    }

    void Game::run()
    {
        on_init();

#if defined(APORIA_EMSCRIPTEN)
        emscripten_set_main_loop_arg(Aporia::main_loop, this, 0, true);
#else
        while (_window.is_open())
        {
            main_loop();
        }
#endif

        on_terminate();
    }

    void Game::main_loop()
    {
        _dt = _timer.reset();

        _window.poll_events();
        _scenes.get_current_scene()->on_input(_inputs);

        _scenes.get_current_scene()->on_scene_transition(_scenes);

        _imgui_layer.begin();
        _renderer.begin(_window, _camera.get_camera());

        on_update(_dt);
        _scenes.get_current_scene()->on_update(_dt);

        _scenes.get_current_scene()->on_draw(_renderer);

        _renderer.end(_camera.get_clear_color());
        _imgui_layer.end();

        _window.display();

        _inputs.update();
    }
}

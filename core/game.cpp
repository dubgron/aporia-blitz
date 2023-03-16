#include "game.hpp"

#if defined(APORIA_EMSCRIPTEN)
    #include <emscripten.h>
#endif

#include "aporia_rendering.hpp"
#include "common.hpp"
#include "scene.hpp"

namespace Aporia
{
    void main_loop(void* game_ptr)
    {
        reinterpret_cast<Aporia::Game*>(game_ptr)->main_loop();
    }

    Game::Game(const std::string& config_file)
        : _window(_camera),
          _imgui_layer(_window)
    {
        load_config(config_file);

        _camera.init(camera_config);
        _window.init(window_config);

        set_default_shader_properties(shader_config.default_properties);
        rendering_init(window_config.width, window_config.height);
        _world.init();

        _layer_stack.push_overlay(_imgui_layer);
    }

    Game::~Game()
    {
        _layer_stack.pop_overlay(_imgui_layer);
        _world.deinit();
        rendering_deinit();
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

        _scenes.get_current_scene()->on_scene_transition(_scenes);

        _imgui_layer.begin();
        rendering_begin(_window, _camera.get_camera());

        on_update(_dt);
        _scenes.get_current_scene()->on_update(_dt);

        rendering_end(_camera.get_clear_color());
        _imgui_layer.end();

        _window.display();

        inputs_update();
    }
}

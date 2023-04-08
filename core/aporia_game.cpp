#include "aporia_game.hpp"

#if defined(APORIA_EMSCRIPTEN)
    #include <emscripten.h>
#endif

#include "aporia_debug.hpp"
#include "aporia_rendering.hpp"

namespace Aporia
{
    void main_loop(void* game_ptr)
    {
        reinterpret_cast<Aporia::Game*>(game_ptr)->main_loop();
    }

    Game::Game(const std::string& config_file)
        : _imgui_layer(_window)
    {
        load_config(config_file);

        _camera.init();
        _window.init(_camera);

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

        frame_timer.reset();

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
        const f32 frame_time = frame_timer.reset();
        total_time += frame_time;

        accumulated_frame_time += frame_time;
        while (accumulated_frame_time > delta_time)
        {
            inputs_update();
            _window.poll_events();

            on_update(total_time, delta_time);
            accumulated_frame_time -= delta_time;
        }

        _imgui_layer.begin();
        rendering_begin(_window, _camera);

        on_draw();

        rendering_end();
        _imgui_layer.end();

        _window.display();
    }
}

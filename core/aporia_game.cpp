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
    {
        load_config(config_file);

        _camera.init();
        _window.init(_camera);

        set_default_shader_properties(shader_config.default_properties);
        rendering_init(window_config.width, window_config.height);
        _world.init();

        imgui_init(_window);
    }

    Game::~Game()
    {
        imgui_deinit();
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

        _window.poll_events();
        poll_gamepad_inputs();

        accumulated_frame_time += frame_time;
        while (accumulated_frame_time > delta_time)
        {
            on_update(total_time, delta_time);
            accumulated_frame_time -= delta_time;

            inputs_clear();
        }

        imgui_frame_begin();
        rendering_begin(_window, _camera);

        on_draw();

        rendering_end();
        imgui_frame_end();

        _window.display();
    }
}

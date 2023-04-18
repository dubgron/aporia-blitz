#include "aporia_game.hpp"

#if defined(APORIA_EMSCRIPTEN)
    #include <emscripten.h>
#endif

#include "aporia_camera.hpp"
#include "aporia_config.hpp"
#include "aporia_debug.hpp"
#include "aporia_rendering.hpp"
#include "aporia_window.hpp"

namespace Aporia
{
    void main_loop(void* game_ptr)
    {
        reinterpret_cast<Aporia::Game*>(game_ptr)->main_loop();
    }

    Game::Game(const std::string& config_file)
    {
        persistent_arena.alloc(MEGABYTES(100));
        frame_arena.alloc(KILOBYTES(10));

        load_config(config_file);

        active_camera = create_camera(&persistent_arena);
        active_window = create_window(&persistent_arena);

        opengl_init();

        set_default_shader_properties(shader_config.default_properties);
        rendering_init(window_config.width, window_config.height);
        world.init();

        imgui_init();
    }

    Game::~Game()
    {
        imgui_deinit();
        world.deinit();
        rendering_deinit();

        destroy_active_window();
    }

    void Game::run()
    {
        on_init();

        frame_timer.reset();

#if defined(APORIA_EMSCRIPTEN)
        emscripten_set_main_loop_arg(Aporia::main_loop, this, 0, true);
#else
        while (active_window->is_open())
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

        frame_arena.clear();

        active_window->poll_events();
        poll_gamepad_inputs();

        accumulated_frame_time += frame_time;
        while (accumulated_frame_time > delta_time)
        {
            on_update(total_time, delta_time);
            accumulated_frame_time -= delta_time;

            inputs_clear();
        }

        imgui_frame_begin();
        rendering_begin();

        on_draw();

        rendering_end();
        imgui_frame_end();

        active_window->display();
    }
}

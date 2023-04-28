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
    MemoryArena persistent_arena;
    MemoryArena frame_arena;

    World world;

    static Game* game = nullptr;

    static Timer frame_timer;
    static f32 total_time = 0.f;

    static f32 delta_time = 1.f / 240.f;
    static f32 accumulated_frame_time = 0.f;

    static void game_main_loop()
    {
        const f32 frame_time = frame_timer.reset();
        total_time += frame_time;

        frame_arena.clear();

        active_window->poll_events();
        poll_gamepad_inputs();

        accumulated_frame_time += frame_time;
        while (accumulated_frame_time > delta_time)
        {
            game->update(total_time, delta_time);
            accumulated_frame_time -= delta_time;

            inputs_clear();
        }

        imgui_frame_begin();
        rendering_begin();

        game->draw(frame_time);

        for (u64 idx = 0; idx < world.entity_count; ++idx)
        {
            const Entity& entity = world.entity_array[idx];
            if (is_flag_set(entity, EntityFlag_Visible))
            {
                draw_entity(entity);
            }
        }

        rendering_end();
        imgui_frame_end();

        active_window->display();
    }

    void game_run(Game* in_game)
    {
        game = in_game;

        // Init
        {
            logging_init("aporia");

            persistent_arena.alloc(MEGABYTES(100));
            frame_arena.alloc(KILOBYTES(10));

            load_config(game->config_filepath);

            active_camera = create_camera(&persistent_arena);
            active_window = create_window(&persistent_arena);

            opengl_init();
            shaders_init();
            rendering_init();

            world.init();

            imgui_init();

            game->init();
        }

        // Update
        {
#if defined(APORIA_EMSCRIPTEN)
            static constexpr auto emscripten_main_loop = [](void* arg) { game_main_loop(); };
            emscripten_set_main_loop_arg(emscripten_main_loop, nullptr, 0, true);
#else
            while (active_window->is_open())
            {
                game_main_loop();
            }
#endif
        }

        // Terminate
        {
            game->terminate();

            imgui_deinit();
            world.deinit();
            rendering_deinit();

            destroy_active_window();
        }
    }
}

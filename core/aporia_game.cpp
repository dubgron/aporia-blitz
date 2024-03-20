#include "aporia_game.hpp"

#if defined(APORIA_EMSCRIPTEN)
    #include <emscripten.h>
#endif

#include "aporia_animations.hpp"
#include "aporia_assets.hpp"
#include "aporia_camera.hpp"
#include "aporia_config.hpp"
#include "aporia_debug.hpp"
#include "aporia_rendering.hpp"
#include "aporia_window.hpp"

namespace Aporia
{
    GameMemory memory;
    World world;

    static Timer frame_timer;
    static f32 total_time = 0.f;

    static f32 delta_time = 1.f / 240.f;
    static f32 accumulated_frame_time = 0.f;

    static void game_initialize()
    {
    }

    static void game_simulate_ui(f32 time, f32 delta_time)
    {
    }

    static void game_simulate_frame(f32 time, f32 delta_time)
    {
    }

    static void game_draw_frame(f32 frame_time)
    {
        for (u64 idx = 0; idx < world.entity_count; ++idx)
        {
            Entity* entity = &world.entity_array[idx];
            if (entity_flag_is_set(*entity, EntityFlag_Visible))
            {
                animation_tick(entity, frame_time);
                draw_entity(*entity);
            }
        }
    }

    static void game_draw_ui(f32 frame_time)
    {
    }

    static void game_terminate()
    {
    }

    static void game_main_loop()
    {
        f32 frame_time = frame_timer.reset();
        total_time += frame_time;

        arena_clear(&memory.frame);

        active_window->poll_events();
        input_process_events();

        IMGUI_FRAME_BEGIN();

        accumulated_frame_time += frame_time;
        while (accumulated_frame_time > delta_time)
        {
            game_simulate_ui(total_time, delta_time);
            game_simulate_frame(total_time, delta_time);

            input_clear();

            assets_reload_if_dirty(delta_time);

            accumulated_frame_time -= delta_time;
        }

        rendering_frame_begin();
        {
            game_draw_frame(frame_time);
        }
        rendering_frame_end();

        rendering_ui_begin();
        {
            game_draw_ui(frame_time);
        }
        rendering_ui_end();

        rendering_flush_to_screen();

        IMGUI_FRAME_END();

        active_window->display();
    }

    void engine_main(String config_filepath)
    {
        // Init
        {
            memory.persistent = arena_init(MEGABYTES(100));
            memory.frame = arena_init(MEGABYTES(1));
            memory.config = arena_init(KILOBYTES(10));

            temporary_memory_init(MEGABYTES(10));

            LOGGING_INIT(&memory.persistent, "aporia");

            assets_init();

            bool config_loaded_successfully = load_engine_config(config_filepath);
            APORIA_ASSERT(config_loaded_successfully);

            active_window = create_window(&memory.persistent);
            active_camera = create_camera(&memory.persistent);

            opengl_init();
            shaders_init(&memory.persistent);
            rendering_init(&memory.persistent);
            animations_init(&memory.persistent);

            world = world_init();

            IMGUI_INIT();

            game_initialize();
        }

        // Update
        {
#if defined(APORIA_EMSCRIPTEN)
            emscripten_set_main_loop(game_main_loop, 0, true);
#else
            while (active_window->is_open())
            {
                game_main_loop();
            }
#endif
        }

        // Terminate
        {
            game_terminate();

            LOGGING_DEINIT();

            return;

            // @NOTE(dubgron): All of the below is optional. Since we're exiting the program,
            // the OS will free this memory for us. Doing it manually has no benefits and it
            // slows down the shutdown of the engine.

            IMGUI_DEINIT();

            world_deinit(&world);
            rendering_deinit();

            destroy_active_window();

            assets_deinit();

            temporary_memory_deinit();

            arena_deinit(&memory.config);
            arena_deinit(&memory.frame);
            arena_deinit(&memory.persistent);
        }
    }
}

int main(int argc, char** argv)
{
    Aporia::engine_main("content/settings.aporia-config");
    return 0;
}

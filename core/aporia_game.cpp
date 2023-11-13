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

    static Game* game = nullptr;

    static Timer frame_timer;
    static f32 total_time = 0.f;

    static f32 delta_time = 1.f / 240.f;
    static f32 accumulated_frame_time = 0.f;

    static void game_main_loop()
    {
        const f32 frame_time = frame_timer.reset();
        total_time += frame_time;

        memory.frame.clear();

        assets_reload_if_dirty();

        active_window->poll_events();
        poll_gamepad_inputs();

        accumulated_frame_time += frame_time;
        while (accumulated_frame_time > delta_time)
        {
            game->update(total_time, delta_time);
            accumulated_frame_time -= delta_time;

            inputs_clear();
        }

        IMGUI_FRAME_BEGIN();
        rendering_begin();

        game->draw(frame_time);

        for (u64 idx = 0; idx < world.entity_count; ++idx)
        {
            Entity& entity = world.entity_array[idx];
            if (is_flag_set(entity, EntityFlag_Visible))
            {
                animation_tick(entity, frame_time);
                draw_entity(entity);
            }
        }

        rendering_end();
        IMGUI_FRAME_END();

        active_window->display();
    }

    void game_run(Game* in_game)
    {
        game = in_game;

        // Init
        {
            memory.persistent.alloc(MEGABYTES(100));
            memory.frame.alloc(MEGABYTES(1));

            for (u64 idx = 0; idx < ARRAY_COUNT(memory.temp); ++idx)
            {
                memory.temp[idx].alloc(MEGABYTES(1));
            }

            memory.config.alloc(KILOBYTES(10));

            LOGGING_INIT(&memory.persistent, "aporia");

            assets_init();

            const bool config_loaded_successfully = load_engine_config(game->config_filepath);
            APORIA_ASSERT(config_loaded_successfully);

            active_window = create_window(&memory.persistent);
            active_camera = create_camera(&memory.persistent);

            opengl_init();
            shaders_init(&memory.persistent);
            rendering_init(&memory.persistent);
            animations_init(&memory.persistent);

            world.init();

            IMGUI_INIT();

            game->init();
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
            game->terminate();

            IMGUI_DEINIT();
            world.deinit();
            rendering_deinit();

            destroy_active_window();

            assets_deinit();

            LOGGING_DEINIT();

            memory.persistent.dealloc();
            memory.frame.dealloc();
            memory.config.dealloc();
        }
    }
}

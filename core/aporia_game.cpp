#include "aporia_game.hpp"

#if defined(APORIA_EMSCRIPTEN)
    #include <emscripten.h>
#endif

#include "aporia_animations.hpp"
#include "aporia_assets.hpp"
#include "aporia_audio.hpp"
#include "aporia_camera.hpp"
#include "aporia_config.hpp"
#include "aporia_debug.hpp"
#include "aporia_rendering.hpp"
#include "aporia_window.hpp"

#if defined(APORIA_EDITOR)
#include "editor/aporia_editor.hpp"
#endif

GameMemory memory;
World world;

static void game_initialize()
{
}

static void game_handle_input(f32 frame_time)
{
}

static void game_simulate_frame(f32 time, f32 delta_time)
{
}

static void game_draw_ui(f32 frame_time)
{
}

static void game_draw_frame(f32 frame_time)
{
}

static void game_shutdown()
{
}

static Timer frame_timer;
static f32 total_time = 0.f;
static f32 game_time = 0.f;

static f32 delta_time = 1.f / 60.f;
static f32 accumulated_frame_time = 0.f;

static void game_main_loop()
{
    f32 frame_time = frame_timer.reset();
    total_time += frame_time;

    arena_clear(&memory.frame);

    input_clear();

    window_poll_events();
    input_process_events();

    assets_reload_if_dirty(frame_time);

    IMGUI_FRAME_BEGIN();

#if defined(APORIA_EDITOR)
    editor_update(frame_time);

    if (editor_is_open)
    {
        frame_time = 0.f;
    }
#endif

    game_time += frame_time;

    game_handle_input(frame_time);

    accumulated_frame_time += frame_time;
    while (accumulated_frame_time > delta_time)
    {
        world_next_frame(&world);

        game_simulate_frame(game_time, delta_time);
        accumulated_frame_time -= delta_time;
    }

    rendering_frame_begin();
    {
        for (i64 idx = 0; idx < world.entity_count; ++idx)
        {
            Entity* entity = &world.entity_array[idx];
            if (entity_flags_has_all(*entity, EntityFlag_Active | EntityFlag_Visible))
            {
                animation_tick(entity, frame_time);

                if (entity_flags_has_all(*entity, EntityFlag_SkipFrameInterpolation))
                {
                    draw_entity(*entity);
                }
                else
                {
                    f32 alpha = accumulated_frame_time / delta_time;
                    Entity entity_interpolated = entity_lerp(world.entity_array_last_frame[idx], *entity, alpha);
                    draw_entity(entity_interpolated);
                }
            }
        }

        game_draw_frame(frame_time);
    }
    rendering_frame_end();

#if defined(APORIA_EDITOR)
    if (!editor_is_open)
    {
#endif

    rendering_ui_begin();
    {
        game_draw_ui(frame_time);
    }
    rendering_ui_end();

#if defined(APORIA_EDITOR)
    }
#endif

    rendering_flush_to_screen();

    IMGUI_FRAME_END();

    window_display();
}

static void engine_main(String config_filepath)
{
    // Init
    {
        memory.persistent = arena_init(MEGABYTES(100));
        memory.frame = arena_init(MEGABYTES(1));
        memory.config = arena_init(KILOBYTES(10));
        memory.assets = arena_init(KILOBYTES(100)); // @TODO(dubgron): This arena should store the assets.

        temporary_memory_init(MEGABYTES(10));

        LOGGING_INIT(&memory.persistent, "aporia");

        assets_init();

        bool config_loaded_successfully = load_engine_config(config_filepath);
        APORIA_ASSERT(config_loaded_successfully);

        window_create(&memory.persistent);
        active_camera = create_camera(&memory.persistent);

        opengl_init();
        shaders_init(&memory.persistent);
        rendering_init(&memory.persistent);
        animations_init(&memory.persistent);
        audio_init();

        world = world_init();

        IMGUI_INIT();

        game_initialize();
    }

    // Update
    {
        world_next_frame(&world);

#if defined(APORIA_EMSCRIPTEN)
        emscripten_set_main_loop(game_main_loop, 0, true);
#else
        while (window_is_open())
        {
            game_main_loop();
        }
#endif
    }

    // Shutdown
    {
        game_shutdown();

        LOGGING_DEINIT();

        return;

        // @NOTE(dubgron): All of the below is optional. Since we're exiting the program,
        // the OS will free this memory for us. Doing it manually has no benefits and it
        // slows down the shutdown of the engine.

        IMGUI_DEINIT();

        world_deinit(&world);

        audio_deinit();
        rendering_deinit();

        window_destroy();

        assets_deinit();

        temporary_memory_deinit();

        arena_deinit(&memory.assets);
        arena_deinit(&memory.config);
        arena_deinit(&memory.frame);
        arena_deinit(&memory.persistent);
    }
}

int main(int argc, char** argv)
{
    engine_main("content/settings.aporia-config");
    return 0;
}

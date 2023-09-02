#pragma once

#include "aporia_memory.hpp"
#include "aporia_world.hpp"

namespace Aporia
{
    typedef void game_init_t();
    typedef void game_update_t(f32 time, f32 delta_time);
    typedef void game_draw_t(f32 frame_time);
    typedef void game_terminate_t();

    struct Game
    {
        game_init_t* init = nullptr;
        game_update_t* update = nullptr;
        game_draw_t* draw = nullptr;
        game_terminate_t* terminate = nullptr;

        String config_filepath;
    };

    extern MemoryArena persistent_arena;
    extern MemoryArena frame_arena;
    extern MemoryArena config_arena;

    extern World world;

    void game_run(Game* in_game);
}

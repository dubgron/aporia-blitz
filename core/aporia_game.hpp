#pragma once

#include "aporia_memory.hpp"
#include "aporia_world.hpp"

// @TODO(dubgron): The list of bugs found during the LD53:
//
// [ ] In load_texture_atlas, the id variable is static, so new textures
//     override old ones because we only use one texture id.
// [ ] CameraDirtyFlag has invalid flags (e.i. 0x00 and 0x01, it should
//     be 0x01 and 0x02).
// [ ] The order of Count in some Input Enums is wrong, and it return
//     not the last value + 1 but the value of the last alias + 1.
// [ ] Memory leak in load_texture_atlas! Forgot to call Image::unload.
// [ ] Missing static specifier in definition of Input input.
//

// @TODO(dubgron): The list of irritating things:
//
// [ ] Setting width and height of Entity with texture.
// [ ] Lack of is_any_key_pressed function.
// [ ] Can't draw centered text.
//

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

        // @TODO(dubgron): Erradicate all usages of std::string and std::string_view
        std::string_view config_filepath;
    };

    extern MemoryArena persistent_arena;
    extern MemoryArena frame_arena;

    extern World world;

    void game_run(Game* in_game);
}

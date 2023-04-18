#pragma once

#include <string>
#include <memory>

#include "aporia_utils.hpp"
#include "aporia_window.hpp"
#include "aporia_world.hpp"

namespace Aporia
{
    struct Game
    {
        MemoryArena persistent_arena;
        MemoryArena frame_arena;

        World world;

        Timer frame_timer;
        f32 total_time = 0.f;

        f32 delta_time = 1.f / 240.f;
        f32 accumulated_frame_time = 0.f;

        Game(const std::string& config_file);
        virtual ~Game();

        virtual void on_init() {};
        virtual void on_update(f32 time, f32 delta_time) {};
        virtual void on_draw() {};
        virtual void on_terminate() {};

        void run();
        void main_loop();

        friend void main_loop(void* game_ptr);

        // Defined by Client
        friend std::unique_ptr<Game> create_game();
    };
}

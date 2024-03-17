#pragma once

#include "aporia_memory.hpp"
#include "aporia_types.hpp"

struct GLFWwindow;

namespace Aporia
{
    struct Window
    {
        GLFWwindow* handle;

        i32 width = 0;
        i32 height = 0;

        void display();

        void poll_events() const;

        void close();
        bool is_open() const;

        v2 get_mouse_screen_position() const;
        v2 get_mouse_world_position() const;

        void apply_config();
    };

    Window* create_window(MemoryArena* arena);
    void destroy_active_window();

    extern Window* active_window;
}

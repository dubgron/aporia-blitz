#pragma once

#include "aporia_memory.hpp"
#include "aporia_types.hpp"

struct GLFWwindow;

namespace Aporia
{
    struct Window
    {
        GLFWwindow* handle;

        void display();

        void poll_events() const;

        void close();
        bool is_open() const;

        v2_u32 get_size() const;
        v2 get_mouse_position() const;

        void on_config_reload();
    };

    Window* create_window(MemoryArena* arena);
    void destroy_active_window();

    extern Window* active_window;
}

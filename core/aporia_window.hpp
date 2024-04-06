#pragma once

#include "aporia_memory.hpp"
#include "aporia_types.hpp"

struct GLFWwindow;

struct Window
{
    GLFWwindow* handle;

    i32 width = 0;
    i32 height = 0;
};

// @NOTE(dubgron): We should only have one window open, so all window-related
// functions operate on this global pointer.
extern Window* active_window;

void window_create(MemoryArena* arena);
void window_destroy();

void window_display();

void window_poll_events();

void window_close();
bool window_is_open();

void window_apply_config();

v2 get_mouse_screen_position();
v2 get_mouse_world_position();

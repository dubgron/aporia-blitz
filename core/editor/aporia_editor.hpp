#pragma once

#include "aporia_types.hpp"

void editor_update(f32 frame_time);
void editor_draw_frame();

extern bool editor_is_open;
extern f32 time_since_selected;

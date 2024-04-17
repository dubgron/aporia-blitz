#pragma once

#include "aporia_types.hpp"

void editor_update(f32 time, f32 delta_time);
void editor_draw_frame(f32 frame_time);

extern bool editor_is_open;
extern f32 time_since_selected;

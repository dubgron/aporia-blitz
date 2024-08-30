#pragma once

#include "aporia_entity.hpp"
#include "aporia_types.hpp"

void editor_update(f32 frame_time);

void editor_draw_selected_entity();
void editor_draw_gizmos();

extern bool editor_is_open;
extern f32 editor_grid_size;

extern EntityID selected_entity_id;
extern f32 time_since_selected;

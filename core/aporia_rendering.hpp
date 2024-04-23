#pragma once

#include "aporia_entity.hpp"
#include "aporia_fonts.hpp"
#include "aporia_shaders.hpp"
#include "aporia_textures.hpp"

struct LightSource
{
    v2 origin{ 0.f };
    f32 range = 1.f;
    f32 falloff = 2.f;

    v3 color{ 1.f };
    f32 intensity = 1.f;
};

bool is_lighting_enabled();
void enable_lighting();
void disable_lighting();

void add_light_source(LightSource light_source);

void rendering_init(MemoryArena* arena);
void rendering_deinit();

void rendering_frame_begin();
void rendering_frame_end();

void rendering_ui_begin();
void rendering_ui_end();

void rendering_flush_to_screen();

void draw_entity(const Entity& entity);
void draw_rectangle(v2 position, f32 width, f32 height, Color color = Color::White, u32 shader_id = rectangle_shader);
void draw_rectangle(v2 base, v2 right, v2 up, Color color = Color::White, u32 shader_id = rectangle_shader);
void draw_line(v2 begin, v2 end, f32 thickness = 1.f, Color color = Color::White, u32 shader_id = line_shader);
void draw_circle(v2 position, f32 radius, Color color = Color::White, u32 shader_id = circle_shader);
void draw_circle(v2 position, f32 radius, f32 inner_radius, Color color = Color::White, u32 shader_id = circle_shader);
void draw_text(const Text& text);

// @HACK(dubgron): We draw triangle as a quad with a duplicate vertex.
void draw_triangle(v2 p0, v2 p1, v2 p2, Color color = Color::White, u32 shader_id = rectangle_shader);

#if defined(APORIA_EDITOR)
i32 read_editor_index();
void set_editor_index(i32 editor_index);
#endif

extern i32 viewport_width;
extern i32 viewport_height;

extern v2_i32 viewport_offset;

extern i32 game_render_width;
extern i32 game_render_height;

extern i32 ui_render_width;
extern i32 ui_render_height;

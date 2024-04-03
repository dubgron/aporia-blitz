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
void draw_line(v2 begin, v2 end, f32 thickness = 1.f, Color color = Color::White, u32 shader_id = line_shader);
void draw_circle(v2 position, f32 radius, Color color = Color::White, u32 shader_id = circle_shader);
void draw_text(const Text& text);

void get_size_of_render_surface(i32* width, i32* height);

void adjust_framebuffers_to_render_surface();

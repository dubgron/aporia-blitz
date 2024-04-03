#pragma once

#include "aporia_shaders.hpp"
#include "aporia_string.hpp"
#include "aporia_textures.hpp"

struct FontAtlas
{
    f32 font_size = 64.f;
    f32 distance_range = 2.f;

    i64 source = INDEX_INVALID;
};

struct GlyphBounds
{
    f32 left = 0.f;
    f32 bottom = 0.f;
    f32 right = 0.f;
    f32 top = 0.f;
};

struct Glyph
{
    u32 unicode = 0;
    f32 advance = 0.f;

    GlyphBounds plane_bounds;
    GlyphBounds atlas_bounds;
};

struct FontMetrics
{
    f32 em_size = 0.f;
    f32 line_height = 0.f;

    f32 ascender_y = 0.f;
    f32 descender_y = 0.f;

    f32 underline_y = 0.f;
    f32 underline_thickness = 0.f;
};

struct Kerning
{
    u32 unicode_1 = 0;
    u32 unicode_2 = 0;
    f32 advance = 0.f;
};

struct Font
{
    String name;
    FontAtlas atlas;

    Glyph* glyphs = nullptr;
    u64 glyphs_count = 0;

    FontMetrics metrics;

    Kerning* kerning = nullptr;
    u64 kerning_count = 0;
};

enum class TextAlignment : u8
{
    Left,
    Center,
    Right,
};

struct Text
{
    v2 position{ 0.f };
    f32 font_size = 32.f;

    f32 rotation = 0.f;
    v2 center_of_rotation{ 0.f };

    Color color = Color::White;
    u32 shader_id = font_shader;

    String caption;
    Font* font = nullptr;

    TextAlignment alignment = TextAlignment::Left;
};

void load_font(String name, String filepath);
Font* get_font(String name);

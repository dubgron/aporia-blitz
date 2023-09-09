#pragma once

#include <map>

#include "aporia_shaders.hpp"
#include "aporia_strings.hpp"
#include "aporia_textures.hpp"

namespace Aporia
{
    struct FontAtlas
    {
        f32 font_size = 64.f;
        f32 distance_range = 2.f;

        Texture* source;
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

    struct Font
    {
        using Glyphs = std::map<u8, Glyph>;
        using Kerning = std::map<std::pair<u8, u8>, f32>;

        FontAtlas atlas;
        Glyphs glyphs;
        FontMetrics metrics;
        Kerning kerning;
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
        const Font* font = nullptr;

        TextAlignment alignment = TextAlignment::Left;
    };

    void load_font(String name, String filepath);
    const Font& get_font(String name);
}

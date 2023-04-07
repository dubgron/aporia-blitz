#pragma once

#include <filesystem>
#include <map>

#include "aporia_entity.hpp"
#include "aporia_shaders.hpp"
#include "aporia_strings.hpp"
#include "aporia_textures.hpp"

namespace Aporia
{
    struct FontAtlas
    {
        Texture source;

        f32 font_size = 64.f;
        f32 distance_range = 2.f;
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

    struct Text final
    {
        Transform2D transform;
        Color color = Colors::White;
        ShaderID program_id = 0;

        std::string caption;
        const Font* font = nullptr;
    };

    void load_font(String name, std::filesystem::path filepath);
    const Font& get_font(String name);
}

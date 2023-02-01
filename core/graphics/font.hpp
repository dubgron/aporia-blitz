#pragma once

#include <map>
#include <utility>

#include "aporia_types.hpp"
#include "components/texture.hpp"

namespace Aporia
{
    using unicode_t = u8;

    struct Font final
    {
        struct Atlas final
        {
            Texture::Origin origin;

            f32 font_size = 64.f;
            f32 distance_range = 2.f;
        };

        struct GlyphData final
        {
            struct Bounds final
            {
                f32 left = 0.f;
                f32 bottom = 0.f;
                f32 right = 0.f;
                f32 top = 0.f;
            };

            f32 advance = 0.f;

            Bounds plane_bounds;
            Bounds atlas_bounds;
        };

        using Glyphs = std::map<unicode_t, GlyphData>;

        struct Metrics final
        {
            f32 em_size = 0.f;
            f32 line_height = 0.f;

            f32 ascender_y = 0.f;
            f32 descender_y = 0.f;

            f32 underline_y = 0.f;
            f32 underline_thickness = 0.f;
        };

        using Kerning = std::map<std::pair<unicode_t, unicode_t>, f32>;

        Atlas atlas;
        Glyphs glyphs;
        Metrics metrics;
        Kerning kerning;
    };
}

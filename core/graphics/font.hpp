#pragma once

#include <map>
#include <utility>

#include "components/texture.hpp"

namespace Aporia
{
    using unicode_t = uint8_t;

    struct Font final
    {
        struct Atlas final
        {
            Texture::Origin origin;

            float font_size = 64.0f;
            float distance_range = 2.0f;
        };

        struct GlyphData final
        {
            struct Bounds final
            {
                float left = 0.0f;
                float bottom = 0.0f;
                float right = 0.0f;
                float top = 0.0f;
            };

            float advance = 0.0f;

            Bounds plane_bounds;
            Bounds atlas_bounds;
        };

        using Glyphs = std::map<unicode_t, GlyphData>;

        struct Metrics final
        {
            float em_size = 0.0f;
            float line_height = 0.0f;

            float ascender_y = 0.0f;
            float descender_y = 0.0f;

            float underline_y = 0.0f;
            float underline_thickness = 0.0f;
        };

        using Kerning = std::map<std::pair<unicode_t, unicode_t>, float>;

        Atlas atlas;
        Glyphs glyphs;
        Metrics metrics;
        Kerning kerning;
    };
}

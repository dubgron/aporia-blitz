#include "aporia_fonts.hpp"

#include "aporia_config.hpp"
#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_utils.hpp"

namespace Aporia
{
    static constexpr u64 MAX_FONTS = 10;

    static Font fonts[MAX_FONTS];
    static u64 fonts_count = 0;

    // @TODO(dubgron): The arena should be parameterized in the future.
    void load_font(String name, String filepath)
    {
        ScratchArena temp = scratch_begin();

        String png_filepath = replace_extension(temp.arena, filepath, "png");
        String config_filepath = replace_extension(temp.arena, filepath, "aporia-config");

        for (u64 idx = 0; idx < fonts_count; ++idx)
        {
            if (fonts[idx].name == name)
            {
                APORIA_LOG(Warning, "Already loaded font named '%'!", name);
                scratch_end(temp);
                return;
            }
        }

        Font result;
        result.name = name;
        result.atlas.source = find_or_load_texture_index(png_filepath);

        // @HACK(dubgron): By default, we set the filter texture parameter to
        // GL_NEAREST, as it works best with pixelart, but fonts look better
        // with linear filtering. Choosing filter should not require to overwrite
        // it after loading the texture.
        glActiveTexture(GL_TEXTURE0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Config_Property* parsed_file = parse_config_from_file(temp.arena, config_filepath);

        u64 glyphs_count = 0;
        u64 kerning_count = 0;

        // TODO(dubgron): Config properties should have info how many fields each category has.
        for (Config_Property* property = parsed_file; property; property = property->next)
        {
            if (property->category == "glyphs")
            {
                glyphs_count += 1;
            }
            else if (property->category == "kerning")
            {
                kerning_count += 1;
            }
        }

        // @TODO(dubgron): The arena should be parameterized in the future.
        result.glyphs = arena_push_uninitialized<Glyph>(&memory.persistent, glyphs_count);
        result.kerning = arena_push_uninitialized<Kerning>(&memory.persistent, kerning_count);

        for (Config_Property* property = parsed_file; property; property = property->next)
        {
            if (property->category == "atlas")
            {
                if (property->field == "size")
                {
                    result.atlas.font_size = string_to_float(property->literals.first->string);
                }
                else if (property->field == "distance_range")
                {
                    result.atlas.distance_range = string_to_float(property->literals.first->string);
                }
            }
            else if (property->category == "metrics")
            {
                if (property->field == "em_size")
                {
                    result.metrics.em_size = string_to_float(property->literals.first->string);
                }
                else if (property->field == "line_height")
                {
                    result.metrics.line_height = string_to_float(property->literals.first->string);
                }
                else if (property->field == "descender")
                {
                    result.metrics.descender_y = string_to_float(property->literals.first->string);
                }
                else if (property->field == "ascender")
                {
                    result.metrics.ascender_y = string_to_float(property->literals.first->string);
                }
                else if (property->field == "underline_y")
                {
                    result.metrics.underline_y = string_to_float(property->literals.first->string);
                }
                else if (property->field == "underline_thickness")
                {
                    result.metrics.underline_thickness = string_to_float(property->literals.first->string);
                }
            }
            else if (property->category == "glyphs")
            {
                Glyph glyph;

                for (Config_Property* glyph_property = property->inner; glyph_property; glyph_property = glyph_property->next)
                {
                    if (glyph_property->field == "advance")
                    {
                        glyph.advance = string_to_float(glyph_property->literals.first->string);
                    }
                    else if (glyph_property->field == "atlas_bounds")
                    {
                        for (Config_Property* atlas_property = glyph_property->inner; atlas_property; atlas_property = atlas_property->next)
                        {
                            if (atlas_property->field == "bottom")
                            {
                                glyph.atlas_bounds.bottom = string_to_float(atlas_property->literals.first->string);
                            }
                            else if (atlas_property->field == "left")
                            {
                                glyph.atlas_bounds.left = string_to_float(atlas_property->literals.first->string);
                            }
                            else if (atlas_property->field == "right")
                            {
                                glyph.atlas_bounds.right = string_to_float(atlas_property->literals.first->string);
                            }
                            else if (atlas_property->field == "top")
                            {
                                glyph.atlas_bounds.top = string_to_float(atlas_property->literals.first->string);
                            }
                        }
                    }
                    else if (glyph_property->field == "plane_bounds")
                    {
                        for (Config_Property* plane_property = glyph_property->inner; plane_property; plane_property = plane_property->next)
                        {
                            if (plane_property->field == "bottom")
                            {
                                glyph.plane_bounds.bottom = string_to_float(plane_property->literals.first->string);
                            }
                            else if (plane_property->field == "left")
                            {
                                glyph.plane_bounds.left = string_to_float(plane_property->literals.first->string);
                            }
                            else if (plane_property->field == "right")
                            {
                                glyph.plane_bounds.right = string_to_float(plane_property->literals.first->string);
                            }
                            else if (plane_property->field == "top")
                            {
                                glyph.plane_bounds.top = string_to_float(plane_property->literals.first->string);
                            }
                        }
                    }
                    else if (glyph_property->field == "unicode")
                    {
                        glyph.unicode = string_to_int(glyph_property->literals.first->string);
                    }
                }

                result.glyphs[result.glyphs_count] = glyph;
                result.glyphs_count += 1;
            }
            else if (property->category == "kerning")
            {
                Kerning kerning;

                for (Config_Property* kerning_property = property->inner; kerning_property; kerning_property = kerning_property->next)
                {
                    if (kerning_property->field == "advance")
                    {
                        kerning.advance = string_to_float(kerning_property->literals.first->string);
                    }
                    else if (kerning_property->field == "unicode_1")
                    {
                        kerning.unicode_1 = string_to_int(kerning_property->literals.first->string);
                    }
                    else if (kerning_property->field == "unicode_2")
                    {
                        kerning.unicode_2 = string_to_int(kerning_property->literals.first->string);
                    }
                }

                result.kerning[result.kerning_count] = kerning;
                result.kerning_count += 1;
            }
        }

        fonts[fonts_count] = result;
        fonts_count += 1;

        scratch_end(temp);
    }

    const Font* get_font(String name)
    {
        for (i64 idx = 0; idx < fonts_count; ++idx)
        {
            if (fonts[idx].name == name)
            {
                return &fonts[idx];
            }
        }

        APORIA_LOG(Error, "Failed to find font '%'!", name);
        return nullptr;
    }
}

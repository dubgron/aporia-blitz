#include "aporia_fonts.hpp"

#include "aporia_config.hpp"
#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_utils.hpp"

namespace Aporia
{
    static std::unordered_map<String, Font> fonts;

    // @TODO(dubgron): The arena should be parameterized in the future.
    void load_font(String name, String filepath)
    {
        ScratchArena temp = create_scratch_arena(&persistent_arena);

        String png_filepath = replace_extension(temp.arena, filepath, "png");
        String config_filepath = replace_extension(temp.arena, filepath, "aporia-config");

        if (fonts.contains(name))
        {
            APORIA_LOG(Warning, "Already loaded font named '%'!", name);
            rollback_scratch_arena(temp);
            return;
        }

        Font result;
        result.atlas.source = find_or_load_texture(png_filepath);

        // @HACK(dubgron): By default, we set the filter texture parameter to
        // GL_NEAREST, as it works best with pixelart, but fonts look better
        // with linear filtering. Choosing filter should not require to overwrite
        // it after loading the texture.
        glBindTexture(GL_TEXTURE_2D, result.atlas.source->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Config_Property* parsed_file = parse_config_from_file(temp.arena, config_filepath);

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
                u8 glyph_unicode = 0;
                Glyph glyph_data;

                for (Config_Property* glyph_property = property->inner; glyph_property; glyph_property = glyph_property->next)
                {
                    if (glyph_property->field == "advance")
                    {
                        glyph_data.advance = string_to_float(glyph_property->literals.first->string);
                    }
                    else if (glyph_property->field == "atlas_bounds")
                    {
                        for (Config_Property* atlas_property = glyph_property->inner; atlas_property; atlas_property = atlas_property->next)
                        {
                            if (atlas_property->field == "bottom")
                            {
                                glyph_data.atlas_bounds.bottom = string_to_float(atlas_property->literals.first->string);
                            }
                            else if (atlas_property->field == "left")
                            {
                                glyph_data.atlas_bounds.left = string_to_float(atlas_property->literals.first->string);
                            }
                            else if (atlas_property->field == "right")
                            {
                                glyph_data.atlas_bounds.right = string_to_float(atlas_property->literals.first->string);
                            }
                            else if (atlas_property->field == "top")
                            {
                                glyph_data.atlas_bounds.top = string_to_float(atlas_property->literals.first->string);
                            }
                        }
                    }
                    else if (glyph_property->field == "plane_bounds")
                    {
                        for (Config_Property* plane_property = glyph_property->inner; plane_property; plane_property = plane_property->next)
                        {
                            if (plane_property->field == "bottom")
                            {
                                glyph_data.plane_bounds.bottom = string_to_float(plane_property->literals.first->string);
                            }
                            else if (plane_property->field == "left")
                            {
                                glyph_data.plane_bounds.left = string_to_float(plane_property->literals.first->string);
                            }
                            else if (plane_property->field == "right")
                            {
                                glyph_data.plane_bounds.right = string_to_float(plane_property->literals.first->string);
                            }
                            else if (plane_property->field == "top")
                            {
                                glyph_data.plane_bounds.top = string_to_float(plane_property->literals.first->string);
                            }
                        }
                    }
                    else if (glyph_property->field == "unicode")
                    {
                        glyph_unicode = string_to_int(glyph_property->literals.first->string);
                    }
                }

                result.glyphs.emplace(glyph_unicode, glyph_data);
            }
            else if (property->category == "kerning")
            {
                std::pair<u8, u8> key;
                f32 advance = 0.f;

                for (Config_Property* kerning_property = property->inner; kerning_property; kerning_property = kerning_property->next)
                {
                    if (kerning_property->field == "advance")
                    {
                        advance = string_to_float(kerning_property->literals.first->string);
                    }
                    else if (kerning_property->field == "unicode_1")
                    {
                        key.first = string_to_int(kerning_property->literals.first->string);
                    }
                    else if (kerning_property->field == "unicode_2")
                    {
                        key.second = string_to_int(kerning_property->literals.first->string);
                    }
                }

                result.kerning.emplace(key, advance);
            }
        }

        fonts.emplace(name, std::move(result));

        rollback_scratch_arena(temp);
    }

    const Font& get_font(String name)
    {
        APORIA_ASSERT_WITH_MESSAGE(fonts.contains(name),
            "Failed to find font '%'!", name);

        return fonts.find(name)->second;
    }
}

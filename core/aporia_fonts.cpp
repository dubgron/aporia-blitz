#include "aporia_fonts.hpp"

#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_parser.hpp"
#include "aporia_utils.hpp"

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

    ParseTreeNode* parsed_file = parse_from_file(temp.arena, config_filepath);

    u64 glyphs_count = 0;
    u64 kerning_count = 0;

    for (ParseTreeNode* node = parsed_file->child_first; node; node = node->next)
    {
        APORIA_ASSERT(node->type == ParseTreeNode_Category);
        if (node->name == "data")
        {
            for (ParseTreeNode* data_node = node->child_first; data_node; data_node = data_node->next)
            {
                if (data_node->name == "glyphs")
                {
                    APORIA_ASSERT(data_node->type = ParseTreeNode_ArrayOfStructs);
                    glyphs_count = data_node->child_count;
                }
                else if (data_node->name == "kerning")
                {
                    APORIA_ASSERT(data_node->type = ParseTreeNode_ArrayOfStructs);
                    kerning_count = data_node->child_count;
                }
            }
        }
    }

    // @TODO(dubgron): The arena should be parameterized in the future.
    result.glyphs = arena_push_uninitialized<Glyph>(&memory.persistent, glyphs_count);
    result.kerning = arena_push_uninitialized<Kerning>(&memory.persistent, kerning_count);

    for (ParseTreeNode* node = parsed_file->child_first; node; node = node->next)
    {
        APORIA_ASSERT(node->type == ParseTreeNode_Category);
        if (node->name == "atlas")
        {
            for (ParseTreeNode* atlas_node = node->child_first; atlas_node; atlas_node = atlas_node->next)
            {
                if (atlas_node->name == "size")
                {
                    get_value_from_field(atlas_node, &result.atlas.font_size);
                }
                else if (atlas_node->name == "distance_range")
                {
                    get_value_from_field(atlas_node, &result.atlas.distance_range);
                }
            }
        }
        else if (node->name == "metrics")
        {
            for (ParseTreeNode* metrics_node = node->child_first; metrics_node; metrics_node = metrics_node->next)
            {
                if (metrics_node->name == "em_size")
                {
                    get_value_from_field(metrics_node, &result.metrics.em_size);
                }
                else if (metrics_node->name == "line_height")
                {
                    get_value_from_field(metrics_node, &result.metrics.line_height);
                }
                else if (metrics_node->name == "descender")
                {
                    get_value_from_field(metrics_node, &result.metrics.descender_y);
                }
                else if (metrics_node->name == "ascender")
                {
                    get_value_from_field(metrics_node, &result.metrics.ascender_y);
                }
                else if (metrics_node->name == "underline_y")
                {
                    get_value_from_field(metrics_node, &result.metrics.underline_y);
                }
                else if (metrics_node->name == "underline_thickness")
                {
                    get_value_from_field(metrics_node, &result.metrics.underline_thickness);
                }
            }
        }
        else if (node->name == "data")
        {
            for (ParseTreeNode* data_node = node->child_first; data_node; data_node = data_node->next)
            {
                if (data_node->name == "glyphs")
                {
                    APORIA_ASSERT(data_node->type = ParseTreeNode_ArrayOfStructs);
                    for (ParseTreeNode* array_node = data_node->child_first; array_node; array_node = array_node->next)
                    {
                        APORIA_ASSERT(array_node->type == ParseTreeNode_Struct);

                        Glyph glyph;

                        for (ParseTreeNode* glyph_node = array_node->child_first; glyph_node; glyph_node = glyph_node->next)
                        {
                            if (glyph_node->name == "advance")
                            {
                                APORIA_ASSERT(glyph_node->type == ParseTreeNode_Field);
                                get_value_from_field(glyph_node, &glyph.advance);
                            }
                            else if (glyph_node->name == "atlas_bounds")
                            {
                                APORIA_ASSERT(glyph_node->type == ParseTreeNode_Struct);
                                for (ParseTreeNode* atlas_bounds_node = glyph_node->child_first; atlas_bounds_node; atlas_bounds_node = atlas_bounds_node->next)
                                {
                                    if (atlas_bounds_node->name == "bottom")
                                    {
                                        get_value_from_field(atlas_bounds_node, &glyph.atlas_bounds.bottom);
                                    }
                                    else if (atlas_bounds_node->name == "left")
                                    {
                                        get_value_from_field(atlas_bounds_node, &glyph.atlas_bounds.left);
                                    }
                                    else if (atlas_bounds_node->name == "right")
                                    {
                                        get_value_from_field(atlas_bounds_node, &glyph.atlas_bounds.right);
                                    }
                                    else if (atlas_bounds_node->name == "top")
                                    {
                                        get_value_from_field(atlas_bounds_node, &glyph.atlas_bounds.top);
                                    }
                                }
                            }
                            else if (glyph_node->name == "plane_bounds")
                            {
                                APORIA_ASSERT(glyph_node->type == ParseTreeNode_Struct);
                                for (ParseTreeNode* plane_bounds_node = glyph_node->child_first; plane_bounds_node; plane_bounds_node = plane_bounds_node->next)
                                {
                                    if (plane_bounds_node->name == "bottom")
                                    {
                                        get_value_from_field(plane_bounds_node, &glyph.plane_bounds.bottom);
                                    }
                                    else if (plane_bounds_node->name == "left")
                                    {
                                        get_value_from_field(plane_bounds_node, &glyph.plane_bounds.left);
                                    }
                                    else if (plane_bounds_node->name == "right")
                                    {
                                        get_value_from_field(plane_bounds_node, &glyph.plane_bounds.right);
                                    }
                                    else if (plane_bounds_node->name == "top")
                                    {
                                        get_value_from_field(plane_bounds_node, &glyph.plane_bounds.top);
                                    }
                                }
                            }
                            else if (glyph_node->name == "unicode")
                            {
                                APORIA_ASSERT(glyph_node->type == ParseTreeNode_Field);
                                get_value_from_field(glyph_node, &glyph.unicode);
                            }

                        }
                        result.glyphs[result.glyphs_count] = glyph;
                        result.glyphs_count += 1;
                    }
                }
                else if (data_node->name == "kerning")
                {
                    APORIA_ASSERT(data_node->type = ParseTreeNode_ArrayOfStructs);

                    for (ParseTreeNode* array_node = data_node->child_first; array_node; array_node = array_node->next)
                    {
                        APORIA_ASSERT(array_node->type == ParseTreeNode_Struct);

                        Kerning kerning;

                        for (ParseTreeNode* kerning_node = array_node->child_first; kerning_node; kerning_node = kerning_node->next)
                        {
                            APORIA_ASSERT(kerning_node->type == ParseTreeNode_Field);
                            if (kerning_node->name == "advance")
                            {
                                get_value_from_field(kerning_node, &kerning.advance);
                            }
                            else if (kerning_node->name == "unicode_1")
                            {
                                get_value_from_field(kerning_node, &kerning.unicode_1);
                            }
                            else if (kerning_node->name == "unicode_2")
                            {
                                get_value_from_field(kerning_node, &kerning.unicode_2);
                            }
                        }

                        result.kerning[result.kerning_count] = kerning;
                        result.kerning_count += 1;
                    }
                }
            }
        }
    }

    fonts[fonts_count] = result;
    fonts_count += 1;

    scratch_end(temp);
}

Font* get_font(String name)
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

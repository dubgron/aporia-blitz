#include "aporia_fonts.hpp"

#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_utils.hpp"

namespace Aporia
{
    static std::unordered_map<String, Font> fonts;

    void load_font(String name, String filepath)
    {
        ScratchArena temp = create_scratch_arena(&persistent_arena);

        String png_filepath = replace_extension(temp.arena, filepath, "png");
        String json_filepath = replace_extension(temp.arena, filepath, "json");

        if (fonts.contains(name))
        {
            APORIA_LOG(Warning, "Already loaded font named '{}'!", name);
        }
        else
        {
            Image atlas;
            atlas.load(png_filepath);
            APORIA_ASSERT(atlas.is_valid());

            // @TODO(dubgron): Move the OpenGL part of creating texture to a separate function.
            u32 id = 0;
            glGenTextures(1, &id);

            glActiveTexture(GL_TEXTURE0 + id);
            glBindTexture(GL_TEXTURE_2D, id);

            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, atlas.width, atlas.height);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlas.width, atlas.height, GL_RGBA, GL_UNSIGNED_BYTE, atlas.pixels);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glGenerateMipmap(GL_TEXTURE_2D);

            Font result;
            result.atlas.source.id = id;
            result.atlas.source.width = atlas.width;
            result.atlas.source.height = atlas.height;
            result.atlas.source.channels = atlas.channels;

            using json = nlohmann::json;

            String data = read_file(temp.arena, json_filepath);
            APORIA_ASSERT(data.is_valid());

            json font = json::parse<std::string_view>(data);

            result.atlas.font_size                  = font["atlas"]["size"];
            result.atlas.distance_range             = font["atlas"]["distanceRange"];

            result.metrics.em_size                  = font["metrics"]["emSize"];
            result.metrics.line_height              = font["metrics"]["lineHeight"];
            result.metrics.ascender_y               = font["metrics"]["descender"];
            result.metrics.descender_y              = font["metrics"]["ascender"];
            result.metrics.underline_y              = font["metrics"]["underlineY"];
            result.metrics.underline_thickness      = font["metrics"]["underlineThickness"];

            for (const auto& glyph : font["glyphs"])
            {
                Glyph glyph_data;

                glyph_data.advance = glyph["advance"];

                if (glyph.contains("atlasBounds"))
                {
                    glyph_data.atlas_bounds.left    = glyph["atlasBounds"]["left"];
                    glyph_data.atlas_bounds.bottom  = glyph["atlasBounds"]["top"];
                    glyph_data.atlas_bounds.right   = glyph["atlasBounds"]["right"];
                    glyph_data.atlas_bounds.top     = glyph["atlasBounds"]["bottom"];
                }

                if (glyph.contains("planeBounds"))
                {
                    glyph_data.plane_bounds.left    = glyph["planeBounds"]["left"];
                    glyph_data.plane_bounds.bottom  = glyph["planeBounds"]["bottom"];
                    glyph_data.plane_bounds.right   = glyph["planeBounds"]["right"];
                    glyph_data.plane_bounds.top     = glyph["planeBounds"]["top"];
                }

                result.glyphs.emplace(glyph["unicode"], glyph_data);
            }

            for (const auto& kerning : font["kerning"])
            {
                std::pair<u8, u8> key = std::make_pair(kerning["unicode1"], kerning["unicode2"]);
                result.kerning.emplace(key, kerning["advance"]);
            }

            fonts.emplace(name, std::move(result));
        }

        rollback_scratch_arena(temp);
    }

    const Font& get_font(String name)
    {
        APORIA_ASSERT_WITH_MESSAGE(fonts.contains(name),
            "Failed to find font '{}'!", name);

        return fonts.find(name)->second;
    }
}

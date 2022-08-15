#include "font_manager.hpp"

#include <cstdint>
#include <utility>

#include <nlohmann/json.hpp>

#include "common.hpp"
#include "graphics/image.hpp"
#include "platform/opengl.hpp"
#include "utils/read_file.hpp"

namespace Aporia
{
    const Font FontManager::default_font{};

    FontManager::FontManager(Logger& logger)
        : _logger(logger) {}

    void FontManager::load_font(std::string name, std::filesystem::path filepath)
    {
        std::filesystem::path png_filepath = filepath.replace_extension("png");
        std::filesystem::path json_filepath = filepath.replace_extension("json");

        if (_fonts.contains(name))
        {
            APORIA_LOG(_logger, Warning, "Already loaded font named '{}'!", name);
        }
        else if (!std::filesystem::exists(png_filepath))
        {
            APORIA_LOG(_logger, Error, "Can't open '{}' file!", png_filepath.relative_path().string());
        }
        else if (!std::filesystem::exists(json_filepath))
        {
            APORIA_LOG(_logger, Error, "Can't open '{}' file!", json_filepath.relative_path().string());
        }
        else
        {
            Font result;

            Image atlas;
            atlas.load(png_filepath.string());

            ImageData& atlas_data = atlas.get_data();
            result.atlas.origin.width = atlas_data.width;
            result.atlas.origin.height = atlas_data.height;
            result.atlas.origin.channels = atlas_data.channels;

            /* TODO: Move the OpenGL part of creatng texture to a separate function */
            uint32_t id = 0;
            glGenTextures(1, &id);

            glActiveTexture(GL_TEXTURE0 + id);
            glBindTexture(GL_TEXTURE_2D, id);

            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, atlas_data.width, atlas_data.height);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlas_data.width, atlas_data.height, GL_RGBA, GL_UNSIGNED_BYTE, atlas_data.pixels);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glGenerateMipmap(GL_TEXTURE_2D);

            result.atlas.origin.id = id;

            using json = nlohmann::json;

            std::string data = read_file(json_filepath.string());
            json font = json::parse(data);

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
                Font::GlyphData glyph_data;

                glyph_data.advance                  = glyph["advance"];

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

                result.glyphs.try_emplace(glyph["unicode"], std::move(glyph_data));
            }

            for (const auto& kerning : font["kerning"])
            {
                std::pair<unicode_t, unicode_t> key = std::make_pair(kerning["unicode1"], kerning["unicode2"]);
                result.kerning.try_emplace(std::move(key), kerning["advance"]);
            }

            _fonts.try_emplace(std::move(name), std::move(result));
        }
    }

    const Font& FontManager::get(const std::string& name) const
    {
        auto font = _fonts.find(name);
        if (font == _fonts.end())
        {
            APORIA_LOG(_logger, Warning, "There is no font named '{}'!", name);
            return default_font;
        }
        else
        {
            return font->second;
        }
    }
}

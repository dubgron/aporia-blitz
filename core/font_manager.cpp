#include "font_manager.hpp"

#include <cstdint>
#include <utility>

#include <nlohmann/json.hpp>

#include "graphics/image.hpp"
#include "graphics/opengl.hpp"
#include "utils/read_file.hpp"

namespace Aporia
{
    FontManager::FontManager(Logger& logger)
        : _logger(logger)
    {
        _fonts.try_emplace("default", Font{});
    }

    void FontManager::load_font(std::string name, std::filesystem::path filepath)
    {
        std::filesystem::path png_filepath = filepath.replace_extension("png");
        std::filesystem::path json_filepath = filepath.replace_extension("json");

        if (_fonts.contains(name))
        {
            _logger.log(LOG_WARNING) << "Already loaded font named " << name << "!";
        }
        else if (!std::filesystem::exists(png_filepath))
        {
            _logger.log(LOG_ERROR) << "Can't open " << png_filepath.relative_path() << " file!";
        }
        else if (!std::filesystem::exists(json_filepath))
        {
            _logger.log(LOG_ERROR) << "Can't open " << json_filepath.relative_path() << " file!";
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

            glGenTextures(1, &result.atlas.origin.id);

            glActiveTexture(GL_TEXTURE0 + result.atlas.origin.id);
            glBindTexture(GL_TEXTURE_2D, result.atlas.origin.id);

            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, atlas_data.width, atlas_data.height);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlas_data.width, atlas_data.height, GL_RGBA, GL_UNSIGNED_BYTE, atlas_data.pixels);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glGenerateMipmap(GL_TEXTURE_2D);

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

    const Font& FontManager::get(const std::string& name)
    {
        auto font = _fonts.find(name);
        if (font == _fonts.end())
        {
            _logger.log(LOG_WARNING) << "There is no font named " << name << "!";
            return _fonts.at("default");
        }
        else
            return font->second;
    }
}

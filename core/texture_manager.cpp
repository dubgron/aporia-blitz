#include "texture_manager.hpp"

#include <filesystem>
#include <memory>

#include <nlohmann/json.hpp>

#include "graphics/common.hpp"
#include "graphics/image.hpp"
#include "graphics/opengl.hpp"
#include "utils/read_file.hpp"

namespace Aporia
{
    TextureManager::TextureManager(Logger& logger, const TextureConfig& config)
        : _logger(logger)
    {
        using json = nlohmann::json;

        if (!std::filesystem::exists(config.atlas))
            logger.log(LOG_ERROR) << "File '" << config.atlas << "' does not open!";
        else
        {
            std::string data = read_file(config.atlas);
            json texture_json = json::parse(data);

            logger.log(LOG_INFO) << "Opened '" << config.atlas << "' successfully";

            std::string filepath = texture_json["atlas"];

            if (!std::filesystem::exists(filepath))
                logger.log(LOG_ERROR) << "File '" << filepath << "' does not open!";
            else
            {
                uint32_t id;

                Image atlas_image{ filepath };
                auto [pixels, width, height, channels] = atlas_image.get_data();

                logger.log(LOG_INFO) << "Opened '" << filepath << "' successfully";

#               if defined(APORIA_EMSCRIPTEN)
                    glGenTextures(1, &id);

                    glActiveTexture(GL_TEXTURE0 + id);
                    glBindTexture(GL_TEXTURE_2D, id);

                    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#               else
                    glCreateTextures(GL_TEXTURE_2D, 1, &id);

                    glTextureStorage2D(id, 1, GL_RGBA8, width, height);
                    glTextureSubImage2D(id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

                    glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                    glBindTextureUnit(id, id);
#               endif

                Texture::Origin atlas{ static_cast<texture_id>(id), width, height, channels };
                for (auto& texture : texture_json["textures"])
                {
                    std::string name = texture["name"];

                    if (_textures.find(name) != _textures.end())
                        logger.log(LOG_WARNING) << "There are two textures named '" << name << "'! One of them will be overwritten!";

                    glm::vec2 u = { texture["u"][0], texture["u"][1] };
                    glm::vec2 v = { texture["v"][0], texture["v"][1] };

                    _textures.try_emplace(name, Texture{ u, v, atlas });
                }

                if (_textures.find("default") == _textures.end())
                {
                    logger.log(LOG_WARNING) << "There is no default texture in '" << filepath << "'!";
                    _textures.try_emplace("default", Texture{ { 0.0f, atlas.height }, { atlas.width, 0.0f }, atlas });
                }

                logger.log(LOG_INFO) << "All textures from '" << filepath << "' loaded successfully";
            }
        }
    }

    const Texture& TextureManager::get_texture(const std::string& name) const
    {
        auto texture = _textures.find(name);
        if (texture == _textures.end())
        {
            _logger.log(LOG_WARNING) << "There is no texture named " << name << "!";
            return _textures.at("default");
        }
        else
            return texture->second;
    }
}

#include "texture_manager.hpp"

#include <filesystem>
#include <memory>

#include <stb_image.h>
#include <GL/gl3w.h>
#include <nlohmann/json.hpp>

#include "components/texture.hpp"
#include "utils/read_file.hpp"

namespace Aporia {
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

            std::string atlas_image = texture_json["meta"]["image"];

            if (!std::filesystem::exists(atlas_image))
                logger.log(LOG_ERROR) << "File '" << atlas_image << "' does not open!";
            else
            {
                uint32_t id;
                int32_t width, height, channels;

                uint8_t* data = stbi_load(atlas_image.c_str(), &width, &height, &channels, 4);

                logger.log(LOG_INFO) << "Opened '" << atlas_image << "' successfully";

                glCreateTextures(GL_TEXTURE_2D, 1, &id);
                glTextureStorage2D(id, 1, GL_RGBA8, width, height);
                glTextureSubImage2D(id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                glBindTextureUnit(id - 1, id);

                stbi_image_free(data);

                Texture::Origin atlas{ id, width, height, channels };
                for (auto& texture : texture_json["frames"])
                {
                    std::string name = texture["filename"];
                    name.erase(name.find_last_of("."));

                    if (_textures.find(name) != _textures.end())
                        logger.log(LOG_WARNING) << "There are two textures named '" << name << "'! One of them will be overwritten!";

                    auto tex = texture["frame"];
                    float x = tex["x"].get<float>() / atlas.width;
                    float y = tex["y"].get<float>() / atlas.height;
                    float width = tex["w"].get<float>() / atlas.width;
                    float height = tex["h"].get<float>() / atlas.height;

                    _textures.try_emplace(name, Texture{ { x, y + height }, { x + width, y }, atlas });
                }

                if (_textures.find("default") == _textures.end())
                {
                    logger.log(LOG_WARNING) << "There is no default texture in '" << atlas_image << "'!";
                    _textures.try_emplace("default", Texture{ { 0.0f, atlas.height }, { atlas.width, 0.0f }, atlas });
                }

                logger.log(LOG_INFO) << "All textures from '" << atlas_image << "' loaded successfully";
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

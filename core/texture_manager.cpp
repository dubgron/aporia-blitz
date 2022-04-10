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
    TextureManager::TextureManager(Logger& logger, EventManager& event_manager, const TextureConfig& config)
        : _logger(logger), _config(config)
    {
        _load();

        event_manager.add_listener<ReloadTextureConfigEvent>(std::bind(&TextureManager::_load, this));
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

    void TextureManager::_load()
    {
        using json = nlohmann::json;

        if (!std::filesystem::exists(_config.atlas))
            _logger.log(LOG_ERROR) << "File '" << _config.atlas << "' does not open!";
        else
        {
            std::string data = read_file(_config.atlas);
            json texture_json = json::parse(data);

            _logger.log(LOG_INFO) << "Opened '" << _config.atlas << "' successfully";

            std::string filepath = texture_json["atlas"];

            if (!std::filesystem::exists(filepath))
                _logger.log(LOG_ERROR) << "File '" << filepath << "' does not open!";
            else
            {
                static uint32_t id = 0u;
                glDeleteTextures(1, &id);

                Image atlas_image{ filepath };
                auto [pixels, width, height, channels] = atlas_image.get_data();

                _logger.log(LOG_INFO) << "Opened '" << filepath << "' successfully";

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

                    glGenerateMipmap(GL_TEXTURE_2D);
#               else
                    glCreateTextures(GL_TEXTURE_2D, 1, &id);

                    glBindTextureUnit(id, id);

                    glTextureStorage2D(id, 1, GL_RGBA8, width, height);
                    glTextureSubImage2D(id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

                    glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                    glGenerateTextureMipmap(id);
#               endif

                _textures.clear();

                Texture::Origin atlas{ static_cast<texture_id>(id), width, height, channels };
                for (auto& texture : texture_json["textures"])
                {
                    std::string name = texture["name"];

                    if (_textures.find(name) != _textures.end())
                        _logger.log(LOG_WARNING) << "There are two textures named '" << name << "'! One of them will be overwritten!";

                    glm::vec2 u = { texture["u"][0], texture["u"][1] };
                    glm::vec2 v = { texture["v"][0], texture["v"][1] };

                    _textures.try_emplace(name, Texture{ u, v, atlas });
                }

                if (_textures.find("default") == _textures.end())
                {
                    _logger.log(LOG_WARNING) << "There is no default texture in '" << filepath << "'!";
                    _textures.try_emplace("default", Texture{ { 0.0f, 1.0f }, { 1.0f, 0.0f }, atlas });
                }

                _logger.log(LOG_INFO) << "All textures from '" << filepath << "' loaded successfully";
            }
        }
    }
}

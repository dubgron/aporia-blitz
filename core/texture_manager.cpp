#include "texture_manager.hpp"

#include <filesystem>
#include <memory>

#include <nlohmann/json.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "utils/read_file.hpp"

namespace Aporia {
    TextureManager::TextureManager(Logger& logger)
        : _logger(logger)
    {
    }

    bool TextureManager::load_textures(const TextureConfig& config)
    {
        using json = nlohmann::json;

        if (!std::filesystem::exists(config.atlas)) 
        {
            _logger.log(LOG_ERROR) << "File '" << config.atlas << "' does not open!";
            return false;
        }

        std::string data = read_file(config.atlas);
        json texture_json = json::parse(data);

        _logger.log(LOG_INFO) << "Opened '" << config.atlas << "' successfully";

        std::string atlas_image = texture_json["meta"]["image"];

        if (!std::filesystem::exists(atlas_image))
        {
            _logger.log(LOG_ERROR) << "File '" << atlas_image << "' does not open!";
            return false;
        }

        std::shared_ptr<sf::Texture> atlas = std::make_shared<sf::Texture>();
        atlas->loadFromFile(atlas_image);
        _logger.log(LOG_INFO) << "Opened '" << atlas_image << "' successfully";

        for (auto& texture : texture_json["frames"]) 
        {
            std::string name = texture["filename"];
            name.erase(name.find_last_of("."));

            if (_textures.find(name) != _textures.end())
            {
                _logger.log(LOG_WARNING) << "There are two textures named '" << name << "'! One of them will be overwritten!";
            }

            auto tex = texture["frame"];
            _textures.try_emplace(name, Texture{ tex["x"], tex["y"], tex["w"], tex["h"], atlas });
        }

        if (_textures.find("default") == _textures.end())
        {
            _logger.log(LOG_WARNING) << "There is no default texture in '" << atlas_image << "'!";
            _textures.try_emplace("default", Texture{ 0, 0, 0, 0, atlas });
        }

        _logger.log(LOG_INFO) << "All textures from '" << atlas_image << "' loaded successfully";
        return true;
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

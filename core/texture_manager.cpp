#include "texture_manager.hpp"

#include <filesystem>
#include <string>

#include <nlohmann/json.hpp>
#include <SFML/Graphics/Rect.hpp>

#include "utils/read_file.hpp"

namespace Aporia {
	TextureManager::TextureManager(const std::shared_ptr<Logger>& logger, const TextureData& config)
		: _logger(logger), _config(config)
	{	
	}

	bool TextureManager::load_textures()
	{
		using json = nlohmann::json;

		std::shared_ptr<sf::Texture> _atlas;

		if (!std::filesystem::exists(_config.atlas)) 
		{
			_logger->log(LOG_ERROR) << "File '" << _config.atlas << "' does not open!";
			return false;
		}

		std::string data = Utils::read_file(_config.atlas);
		json texture_json = json::parse(data);

		_logger->log(LOG_INFO) << "Opened '" << _config.atlas << "' successfully";

		std::string atlas_image = texture_json["meta"]["image"];

		if (!std::filesystem::exists(atlas_image))
		{
			_logger->log(LOG_ERROR) << "File '" << atlas_image << "' does not open!";
			return false;
		}

		_atlas->loadFromFile(atlas_image);
		_logger->log(LOG_INFO) << "Opened '" << atlas_image << ".png' successfully";

		for (auto& texture : texture_json["frames"]) 
		{
			std::string name = texture["filename"];
			
			name.erase(name.find_last_of("."));

			if (_textures.find(name) != _textures.end())
			{
				_logger->log(LOG_WARNING) << "There are two textures called '" << name << "'! One of them will be overwritten!";
			}

			auto tex = texture["frame"];
			_textures.try_emplace(name, std::make_shared<Texture>(tex["x"], tex["y"], tex["w"], tex["h"], _atlas));
		}

		if (_textures.find("default") == _textures.end())
		{
			_logger->log(LOG_WARNING) << "There is no default texture in '" << atlas_image << "'!";
			_textures.try_emplace("default", std::make_shared<Texture>(0, 0, 0, 0, _atlas));
		}

		_logger->log(LOG_INFO) << "All textures from '" << atlas_image << "' loaded successfully";
		return true;
	}

	std::shared_ptr<Texture>& TextureManager::get_sprite(const std::string& name)
	{
		auto texture = _textures.find(name);
		return (texture != _textures.end() ? texture->second : _textures["default"]);
	}
}
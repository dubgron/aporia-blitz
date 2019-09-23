#include "texture_manager.hpp"

#include <filesystem>
#include <string>

#include <nlohmann/json.hpp>

#include "utils/read_file.hpp"

namespace Aporia {
	TextureManager::TextureManager(std::shared_ptr<Logger> logger)
		: _logger(logger)
	{	
	}

	bool TextureManager::load_textures(const std::string& atlas_name)
	{
		using json = nlohmann::json;

		if (!std::filesystem::exists(atlas_name + ".png"))
		{
			_logger->log(LOG_ERROR) << "File '" << atlas_name << ".png' does not open!";
			return false;
		}
		_atlas.loadFromFile(atlas_name + ".png");
		_logger->log(LOG_INFO) << "Opened '" << atlas_name << ".png' successfully";

		if (!std::filesystem::exists(atlas_name + ".json")) 
		{
			_logger->log(LOG_ERROR) << "File '" << atlas_name << ".json' does not open!";
			return false;
		}
		std::string data = Utils::read_file(atlas_name + ".json");
		json texture_json = json::parse(data);
		_logger->log(LOG_INFO) << "Opened '" << atlas_name << ".json' successfully";

		for (auto& texture : texture_json["frames"]) 
		{
			std::string name = texture["filename"];

			if (_textures.find(name) != _textures.end())
			{
				_logger->log(LOG_WARNING) << "There are two textures called '" << name << "'! One of them will be overwritten!";
			}

			name.erase(name.find_last_of("."));

			_textures[name].x = texture["frame"]["x"];
			_textures[name].y = texture["frame"]["y"];
			_textures[name].width = texture["frame"]["w"];
			_textures[name].height = texture["frame"]["h"];
		}

		if (_textures.find("default") == _textures.end())
		{
			_logger->log(LOG_WARNING) << "There is no default texture in '" << atlas_name << ".png'!";
			/* TODO: Set default texture, while atlas doesn't have one */
			//_textures["default"] = 
		}

		_logger->log(LOG_INFO) << "All textures from '" << atlas_name << "' loaded successfully";
		return true;
	}

	const Texture_info& TextureManager::get_texture_info(const std::string& name)
	{
		auto texture_info = _textures.find(name);
		return texture_info->second;
	}

	const sf::Texture& TextureManager::get_atlas()
	{
		return _atlas;
	}
}
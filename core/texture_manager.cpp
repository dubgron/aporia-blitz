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

		sf::Image atlas;
		if (!std::filesystem::exists(atlas_name + ".png"))
		{
			_logger->log(LOG_ERROR) << "File '" << atlas_name << ".png' does not open!";
			return false;
		}
		atlas.loadFromFile(atlas_name + ".png");
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
			unsigned int x = texture["frame"]["x"];
			unsigned int y = texture["frame"]["y"];
			unsigned int width = texture["frame"]["w"];
			unsigned int height = texture["frame"]["h"];

			name.erase(name.end() - 3);

			if (_textures.find(name) != _textures.end())
			{
				_logger->log(LOG_WARNING) << "There are two textures called '" << name << "'! One of them will be overwritten!";
			}

			_textures[name].loadFromImage(atlas, sf::IntRect(x, y, width, height));
		}

		if (_textures.find("default") == _textures.end())
		{
			_logger->log(LOG_WARNING) << "There is no default texture in '" << atlas_name << ".png'!";
			_textures["default"] = sf::Texture();
		}

		_logger->log(LOG_INFO) << "All textures from '" << atlas_name << "' loaded successfully";
		return true;
	}

	const sf::Texture& TextureManager::get_texture(const std::string& name)
	{
		auto texture = _textures.find(name);
		return (texture != _textures.end() ? texture->second : _textures["default"]);
	}
}
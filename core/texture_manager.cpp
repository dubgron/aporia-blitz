#include "texture_manager.hpp"

#include <filesystem>
#include <fstream>

namespace Aporia {
	TextureManager::TextureManager(std::shared_ptr<Logger> logger)
		: _logger(logger)
	{	
	}

	bool TextureManager::load_textures(const std::string& atlas_name)
	{
		sf::Image atlas;
		if (!std::filesystem::exists(atlas_name + ".png"))
		{
			_logger->log(LOG_ERROR) << "File '" << atlas_name << ".png' does not open!";
			return false;
		}
		atlas.loadFromFile(atlas_name + ".txt");
		_logger->log(LOG_INFO) << "Opened '" << atlas_name << ".png' successfully";

		std::ifstream texture_data(atlas_name + ".txt");
		if (texture_data.fail()) 
		{
			_logger->log(LOG_ERROR) << "File '" << atlas_name << ".txt' does not open!";
			return false;
		}
		_logger->log(LOG_INFO) << "Opened '" << atlas_name << ".txt' successfully";

		while (texture_data.good())
		{
			std::string path;
			int x, y, width, height;

			texture_data >> path;
			texture_data >> x >> y >> width >> height;

			std::string name = path.substr(path.find_last_of('/') + 1);

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
		texture_data.close();
		return true;
	}

	const sf::Texture& TextureManager::get_texture(const std::string& name)
	{
		auto texture = _textures.find(name);
		return (texture != _textures.end() ? texture->second : _textures["default"]);
	}
}
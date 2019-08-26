#pragma once

#include <iostream>
#include <string>
#include <map>

#include <SFML/Graphics/Texture.hpp>

#include "platform.hpp"
#include "logger.hpp"

namespace Aporia
{
	class TextureManager 
	{
	public:
		TextureManager(std::shared_ptr<Logger> logger);

		bool load_textures(const std::string& atlas_name);

		const sf::Texture& get_texture(const std::string& name);

	private:
		std::shared_ptr<Logger> _logger;
		std::map<std::string, sf::Texture> _textures;
	};
}

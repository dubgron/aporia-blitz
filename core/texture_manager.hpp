#pragma once

#include <iostream>
#include <string>
#include <map>

#include <SFML/Graphics/Texture.hpp>

#include "platform.hpp"
#include "logger.hpp"

namespace Aporia
{
	struct Texture_info {
		float x;
		float y;
		float width;
		float height;
	};

	class TextureManager 
	{
	public:
		TextureManager(std::shared_ptr<Logger> logger);

		bool load_textures(const std::string& atlas_name);

		const Texture_info& get_texture_info(const std::string& name);
		const sf::Texture& get_atlas();

	private:
		std::shared_ptr<Logger> _logger;
		std::map<std::string, Texture_info> _textures;
		sf::Texture _atlas;
	};
}

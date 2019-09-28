#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <SFML/Graphics/Texture.hpp>

#include "configs/texture_config.hpp"
#include "logger.hpp"
#include "platform.hpp"

namespace Aporia
{
	struct APORIA_API Texture
	{
		unsigned int x, y;
		unsigned width, height;

		std::shared_ptr<sf::Texture> atlas;

		Texture(unsigned int x, unsigned int y, unsigned int width, unsigned int height, std::shared_ptr<sf::Texture> altas)
			: x(x), y(y), width(width), height(height), atlas(atlas) {};
	};

	class APORIA_API TextureManager 
	{
	public:
		TextureManager(const std::shared_ptr<Logger>& logger, const TextureData& config);

		bool load_textures();

		std::shared_ptr<Texture>& get_sprite(const std::string& name);

	private:
		std::shared_ptr<Logger> _logger;

		std::map<std::string, std::shared_ptr<Texture>> _textures;
		const TextureData& _config;
	};
}

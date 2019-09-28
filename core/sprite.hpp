#pragma once

#include <memory>

#include <SFML/System/Vector2.hpp>

#include "logger.hpp"
#include "platform.hpp"
#include "texture_manager.hpp"

namespace Aporia
{
	class APORIA_API Sprite
	{
	public:
		Sprite(const std::shared_ptr<Logger>& logger, std::shared_ptr<Texture> texture, sf::Vector2f position)
			:_logger(logger), _texture(texture), _position(position) {};

		sf::Vector2f get_position()
		{
			return _position;
		}

		const std::shared_ptr<Texture>& get_texture()
		{
			return _texture;
		}

	private:
		std::shared_ptr<Logger> _logger;
		std::shared_ptr<Texture> _texture;
		sf::Vector2f _position;
	};
}
#pragma once

#include <iostream>
#include <string>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "platform.hpp"
#include "logger.hpp"
#include "texture_manager.hpp"

namespace Aporia 
{
	class Renderer : public sf::Drawable, public sf::Transformable
	{
	public:
		Renderer(std::shared_ptr<Logger> logger);

		void add_quad(std::string& name, int x, int y, unsigned int width, unsigned int height);

	private:
		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

		std::shared_ptr<Logger> _logger;
		TextureManager _textureManager;
		unsigned int _size;
		sf::VertexArray _vertices;
		sf::Texture _atlas;	
	};
}
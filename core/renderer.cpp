/*
#include "renderer.hpp"
#include "texture_manager.hpp"

#include <SFML/Graphics/Vertex.hpp>

namespace Aporia
{
	Renderer::Renderer(std::shared_ptr<Logger> logger)
		: _logger(logger), _textureManager(logger)
	{
		_textureManager.load_textures("atlas");
		_atlas = _textureManager.get_atlas();
		_size = 0;
	}

	void Renderer::add_quad(std::string& name, int x, int y, unsigned int width, unsigned int height)
	{
		_size++;

		_vertices.setPrimitiveType(sf::Quads);
		_vertices.resize(_size * 4);

		Texture_info texture_info;
		texture_info = _textureManager.get_texture_info(name);

		sf::Vertex* quad = &_vertices[(_size - 1) * 4];

		quad[0].position = sf::Vector2f(x, y);
		quad[1].position = sf::Vector2f(x + width, y);
		quad[2].position = sf::Vector2f(x + width, y + height);
		quad[3].position = sf::Vector2f(x, y + height);

		quad[0].texCoords = sf::Vector2f(texture_info.x, texture_info.y);
		quad[1].texCoords = sf::Vector2f(texture_info.x + texture_info.width, texture_info.y);
		quad[2].texCoords = sf::Vector2f(texture_info.x +texture_info.width, texture_info.y + texture_info.height);
		quad[3].texCoords = sf::Vector2f(texture_info.x, texture_info.y + texture_info.height);
	}

	void Renderer::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform = getTransform();

		states.texture = &_atlas;

		target.draw(_vertices, states);
	}
}
*/
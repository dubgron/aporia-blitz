#pragma once

#include <array>
#include <vector>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include "platform.hpp"

namespace Aporia
{
    class APORIA_API VertexArray final : public sf::Drawable
    {
    public:
        VertexArray(sf::PrimitiveType primitive_type = sf::Quads, size_t size = 10000);

        void add(const std::array<sf::Vertex, 4>& vertecies);
        void clear();

        size_t size() const;

    private:
        std::vector<sf::Vertex> _vertices;
        sf::PrimitiveType _primitive_type;

        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    };
}

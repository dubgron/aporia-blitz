#pragma once

#include <array>
#include <map>
#include <memory>
#include <type_traits>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include "camera.hpp"
#include "logger.hpp"
#include "vertex_array.hpp"
#include "window.hpp"
#include "components/circular.hpp"
#include "components/color.hpp"
#include "components/linear2d.hpp"
#include "components/rectangular.hpp"
#include "components/texture.hpp"
#include "components/transform2d.hpp"
#include "utils/type_traits.hpp"

namespace Aporia
{
    class Renderer final
    {
        static constexpr std::size_t MAX_QUEUE = 10000;

    public:
        Renderer(Logger& logger);

        template<typename T, std::enable_if_t<has_type_v<typename T::Components, Rectangular>, int> = 0>
        void draw(const T& entity);

        template<typename T, std::enable_if_t<has_type_v<typename T::Components, Circular>, int> = 0>
        void draw(const T& entity);

        template<typename T, std::enable_if_t<has_type_v<typename T::Components, Linear2D>, int> = 0>
        void draw(const T& entity);

        void render(Window& window, const Camera& cam);

    private:
        template<std::size_t N>
        void _apply_color(std::array<sf::Vertex, N>& vertecies, const Color& color);

        template<std::size_t N>
        void _apply_transform2d(std::array<sf::Vertex, N>& vertecies, const Transform2D& transform);

        template<std::size_t N>
        void _apply_texture(std::array<sf::Vertex, N>& vertecies, const Texture& texture);

        std::map<std::shared_ptr<sf::Texture>, VertexArray<IndexBuffer::Quads, MAX_QUEUE>> _textured_queue;
        VertexArray<IndexBuffer::Quads, MAX_QUEUE> _quad_queue;
        VertexArray<IndexBuffer::TriangleFan, MAX_QUEUE> _circle_queue;
        VertexArray<IndexBuffer::Lines, MAX_QUEUE> _line_queue;

        Logger& _logger;
    };
}

#include "renderer.tpp"

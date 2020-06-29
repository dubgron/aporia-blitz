#pragma once

#include <map>
#include <memory>

#include <SFML/Graphics/Texture.hpp>

#include "camera.hpp"
#include "event_manager.hpp"
#include "logger.hpp"
#include "sprite.hpp"
#include "vertex_array.hpp"
#include "window.hpp"

namespace Aporia
{
    class Renderer final
    {
    public:
        Renderer(Logger& logger);

        template<typename T, std::enable_if_t<has_types_v<typename T::Components, Transform2D, Texture>, int> = 0>
        void draw(const T& entity);

        void render(Window& window, const Camera& cam);

    private:
        std::map<std::shared_ptr<sf::Texture>, VertexArray> _queue;

        const size_t _sprites = 10000;

        Logger& _logger;
    };
}

#include "renderer.tpp"

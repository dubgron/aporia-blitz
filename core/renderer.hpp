#pragma once

#include <map>
#include <memory>

#include <SFML/Graphics/Texture.hpp>

#include "event_manager.hpp"
#include "logger.hpp"
#include "platform.hpp"
#include "sprite.hpp"
#include "vertex_array.hpp"
#include "window.hpp"

namespace Aporia
{
    class APORIA_API Renderer final
    {
    public:
        Renderer(Logger& logger, EventManager& event_manager);

        void draw(const Sprite& sprite);
        void render(Window& window);

    private:
        std::map<std::shared_ptr<sf::Texture>, VertexArray> _queue;

        const size_t _sprites = 10000;

        Logger& _logger;
    };
}
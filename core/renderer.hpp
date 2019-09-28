#pragma once

#include <map>
#include <memory>

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
        Renderer(const std::shared_ptr<Logger>& logger);

        void draw(const Sprite& sprite);
        void render(Window& window);

    private:
        std::map<std::shared_ptr<Texture>, VertexArray> _queue;

        const size_t _sprites = 10000;

        std::shared_ptr<Logger> _logger;
    };
}
#pragma once

#include <memory>
#include <tuple>

#include "config_manager.hpp"
#include "event_manager.hpp"
#include "input_manager.hpp"
#include "logger.hpp"
#include "platform.hpp"
#include "sprite.hpp"
#include "texture_manager.hpp"
#include "window.hpp"

namespace Aporia
{
    class APORIA_API Game
    {
        friend class Engine;

    public:
        Game() = default;
        virtual ~Game() = default;

        virtual void on_init() {};
        virtual void on_update() {};
        virtual void on_terminate() {};

    protected:
        void draw(const Sprite& sprite)
        {
            _events->call_event<DrawSpriteEvent>(sprite);
        }

        std::shared_ptr<TextureManager> get_textures() const
        {
            return _textures;
        }

        std::shared_ptr<InputManager> get_inputs() const
        {
            return _inputs;
        }

        std::shared_ptr<Logger> get_logger() const
        {
            return _logger;
        }

    private:
        std::shared_ptr<ConfigManager> _configs;
        std::shared_ptr<EventManager> _events;
        std::shared_ptr<InputManager> _inputs;
        std::shared_ptr<TextureManager> _textures;

        std::shared_ptr<Logger> _logger;

        /* Defined by Client */
        friend std::unique_ptr<Game> create_game();
    };
}

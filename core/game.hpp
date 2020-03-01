#pragma once

#include <string>
#include <memory>
#include <tuple>

#include "config_manager.hpp"
#include "event_manager.hpp"
#include "input_manager.hpp"
#include "logger.hpp"
#include "platform.hpp"
#include "sprite.hpp"
#include "renderer.hpp"
#include "texture_manager.hpp"
#include "window.hpp"

namespace Aporia
{
    class APORIA_API Game
    {
        /* Defined by Client */
        friend std::unique_ptr<Game> create_game();

    public:
        Game(const std::string& config_file);
        virtual ~Game() = default;

        virtual void on_init() {};
        virtual void on_update() {};
        virtual void on_terminate() {};

        void run();

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

        std::unique_ptr<Renderer> _renderer;
        std::shared_ptr<Window> _window;

        std::shared_ptr<Logger> _logger;
    };
}

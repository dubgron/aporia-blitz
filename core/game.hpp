#pragma once

#include <string>
#include <memory>
#include <tuple>

#include "config_manager.hpp"
#include "event_manager.hpp"
#include "input_manager.hpp"
#include "logger.hpp"
#include "sprite.hpp"
#include "renderer.hpp"
#include "texture_manager.hpp"
#include "window.hpp"

namespace Aporia
{
    class Game
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
        Logger _logger;

        ConfigManager _configs;
        EventManager _events;
        InputManager _inputs;
        TextureManager _textures;

        Renderer _renderer;
        Window _window;
    };
}

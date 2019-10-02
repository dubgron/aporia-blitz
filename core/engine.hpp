#pragma once

#include <string>

#include "config_manager.hpp"
#include "event_manager.hpp"
#include "game.hpp"
#include "input_manager.hpp"
#include "logger.hpp"
#include "platform.hpp"
#include "renderer.hpp"
#include "texture_manager.hpp"

namespace Aporia
{
    class APORIA_API Engine final
    {
    public:
        Engine(const std::string& config_file);

        void run(std::unique_ptr<Game> game);

    private:
        std::shared_ptr<ConfigManager> _configs;
        std::shared_ptr<EventManager> _events;
        std::shared_ptr<InputManager> _inputs;
        std::shared_ptr<TextureManager> _textures;

        std::unique_ptr<Renderer> _renderer;

        std::shared_ptr<Logger> _logger;
    };
}

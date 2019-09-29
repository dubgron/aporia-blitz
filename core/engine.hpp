#pragma once

#include <string>

#include "config_manager.hpp"
#include "event_manager.hpp"
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

    private:
        std::unique_ptr<ConfigManager> _configs;
        std::unique_ptr<TextureManager> _textures;
        std::unique_ptr<EventManager> _events;
        std::unique_ptr<Renderer> _renderer;

        std::shared_ptr<Logger> _logger;
    };
}

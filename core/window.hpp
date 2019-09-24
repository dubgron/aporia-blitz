#pragma once

#include <memory>
#include <string>

#include <SFML/Window.hpp>

#include "configs/window_config.hpp"
#include "logger.hpp"
#include "platform.hpp"

namespace Aporia
{
    class APORIA_API Window final
    {
        friend class EventManager;

    public:
        Window(const std::shared_ptr<Logger>& logger, const WindowConfig& config);
        ~Window();

        void open();
        void close();

        void show();
        void hide();

        bool is_open() const;
        bool is_visible() const;

    private:
        std::shared_ptr<Logger> _logger;

        sf::Window _window;
        const WindowConfig& _config;

        bool _visible = false;
    };
}

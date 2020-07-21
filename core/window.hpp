#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

#include "logger.hpp"
#include "configs/window_config.hpp"

namespace Aporia
{
    class Window final
    {
    public:
        Window(Logger& logger, const WindowConfig& config);
        ~Window();

        void open();
        void close();

        void show();
        void hide();

        void update(const sf::Time& delta_time);
        void clear(const sf::Color& color = sf::Color(0, 0, 0, 255));
        void draw(const sf::Drawable& drawable, sf::RenderStates& states);
        void display();

        bool poll_event(sf::Event& event);

        bool is_open() const;
        bool is_visible() const;

    private:
        Logger& _logger;

        sf::RenderWindow _window;
        const WindowConfig& _config;

        bool _visible = false;
    };
}

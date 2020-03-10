#pragma once

#include <memory>
#include <string>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include "logger.hpp"
#include "platform.hpp"
#include "vertex_array.hpp"
#include "configs/window_config.hpp"

namespace Aporia
{
    class APORIA_API Window final
    {
    public:
        Window(Logger& logger, const WindowConfig& config);
        ~Window();

        void open();
        void close();

        void show();
        void hide();

        void clear(const sf::Color& color = sf::Color(0, 0, 0, 255));
        void draw(const VertexArray& vertex_array, sf::RenderStates states);
        void display();

        bool poll_event(sf::Event& event);

        bool is_open() const;
        bool is_visible() const;

        /* Returns sf::Transform that transforms the point (0, 0) to the middle of the screen and flips everything along X axis*/
        const sf::Transform& get_transform() const;

    private:
        Logger& _logger;

        sf::RenderWindow _window;
        const WindowConfig& _config;

        sf::Transform _transform;
        bool _visible = false;
    };
}

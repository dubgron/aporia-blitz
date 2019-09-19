#include "event_manager.hpp"

#include <SFML/Window/Event.hpp>

#include "inputs.hpp"

namespace Aporia
{
    EventManager::EventManager(std::shared_ptr<Logger> logger)
        : _logger(logger)
    {

    }

    void EventManager::listen_for_events(Window& window)
    {
        sf::Event event;

        Keyboard key;
        Mouse button;

        while (window._window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                    call_event<WindowCloseEvent>(_logger, window);
                    break;

                case sf::Event::Resized:
                    call_event<WindowResizeEvent>(_logger, window, event.size.width, event.size.height);
                    break;

                case sf::Event::KeyPressed:
                    key = static_cast<Keyboard>(event.key.code);
                    call_event<KeyPressedEvent>(_logger, key, event.key.alt, event.key.control, event.key.shift, event.key.system);
                    break;

                case sf::Event::KeyReleased:
                    key = static_cast<Keyboard>(event.key.code);
                    call_event<KeyReleasedEvent>(_logger, key, event.key.alt, event.key.control, event.key.shift, event.key.system);
                    break;

                case sf::Event::MouseButtonPressed:
                    button = static_cast<Mouse>(event.mouseButton.button);
                    call_event<ButtonPressedEvent>(_logger, button, event.mouseButton.x, event.mouseButton.y);
                    break;

                case sf::Event::MouseButtonReleased:
                    button = static_cast<Mouse>(event.mouseButton.button);
                    call_event<ButtonReleasedEvent>(_logger, button, event.mouseButton.x, event.mouseButton.y);
                    break;
            }
        }
    }
}

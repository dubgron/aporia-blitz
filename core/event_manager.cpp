#include "event_manager.hpp"

#include <SFML/Window/Event.hpp>

#include "event.hpp"

namespace Aporia
{
    EventManager::EventManager(std::shared_ptr<Logger> logger)
        : _logger(logger)
    {

    }

    void EventManager::listen_for_events(Window& window)
    {
        sf::Event event;

        while (window._window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                    call_event<WindowCloseEvent>(window);
                    break;

                case sf::Event::Resized:
                    call_event<WindowResizeEvent>(window, event.size.width, event.size.height);
                    break;
            }
        }
    }
}

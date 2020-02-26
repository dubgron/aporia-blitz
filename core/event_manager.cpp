#include "event_manager.hpp"

#include <SFML/Window/Event.hpp>

#include "event.hpp"
#include "inputs.hpp"

namespace Aporia
{
    EventManager::EventManager(const std::shared_ptr<Logger>& logger)
        : _logger(logger)
    {

    }

    void EventManager::listen_for_events(Window& window)
    {
        call_event<BeginProcessingWindowEvents>();

        Keyboard key;
        Mouse button;
        MouseWheel wheel;

        sf::Event event;
        while (window.poll_event(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                    call_event<WindowCloseEvent>(window);
                    break;

                case sf::Event::Resized:
                    call_event<WindowResizeEvent>(window, event.size.width, event.size.height);
                    break;

                case sf::Event::KeyPressed:
                    key = static_cast<Keyboard>(1 + event.key.code);
                    call_event<KeyPressedEvent>(key);
                    break;

                case sf::Event::KeyReleased:
                    key = static_cast<Keyboard>(1 + event.key.code);
                    call_event<KeyReleasedEvent>(key);
                    break;

                case sf::Event::MouseButtonPressed:
                    button = static_cast<Mouse>(event.mouseButton.button);
                    call_event<ButtonPressedEvent>(button);
                    break;

                case sf::Event::MouseButtonReleased:
                    button = static_cast<Mouse>(event.mouseButton.button);
                    call_event<ButtonReleasedEvent>(button);
                    break;

                case sf::Event::MouseWheelScrolled:
                    wheel = static_cast<MouseWheel>(event.mouseWheelScroll.wheel);
                    call_event<MouseWheelScrollEvent>(wheel, event.mouseWheelScroll.delta);
                    break;
            }
        }

        call_event<EndProcessingWindowEvents>();
    }
}

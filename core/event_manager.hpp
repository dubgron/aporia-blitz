#pragma once

#include <tuple>
#include <type_traits>

#include "event.hpp"
#include "logger.hpp"
#include "window.hpp"
#include "utils/type_traits.hpp"

namespace Aporia
{
    class EventManager final
    {
        using Events = std::tuple<WindowCloseEvent,
                                  WindowResizeEvent,
                                  KeyPressedEvent,
                                  KeyReleasedEvent,
                                  ButtonPressedEvent,
                                  ButtonReleasedEvent,
                                  MouseWheelScrollEvent,
                                  BeginProcessingWindowEvents,
                                  EndProcessingWindowEvents,
                                  DrawSpriteEvent>;

    public:
        EventManager(Logger& logger);

        void listen_for_events(Window& window);

        template<typename Ev, typename... Args, 
            typename = std::enable_if_t<
                std::is_invocable_v<typename Ev::event_type, Args&...> &&
                is_in_tuple_v<Ev, Events>
            >>
        void call_event(Args&... args);

        template<typename Ev, typename Listener,
            typename = std::enable_if_t<
                std::is_constructible_v<typename Ev::event_type, Listener> &&
                is_in_tuple_v<Ev, Events>
            >>
        void add_listener(Listener listener);

    private:
        Events _events;

        Logger& _logger;
    };
}

#include "event_manager.tpp"

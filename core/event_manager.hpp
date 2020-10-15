#pragma once

#include <tuple>
#include <type_traits>

#include "event.hpp"
#include "logger.hpp"
#include "utils/type_traits.hpp"

namespace Aporia
{
    class Window;

    class EventManager final
    {
        using Events = std::tuple<WindowCloseEvent,
            WindowResizeEvent,
            KeyPressedEvent,
            KeyReleasedEvent,
            ButtonPressedEvent,
            ButtonReleasedEvent,
            MouseWheelScrollEvent,
            MouseMoveEvent,
            BeginProcessingWindowEvents,
            EndProcessingWindowEvents,
            BeginFrameEvent,
            EndFrameEvent>;

    public:
        EventManager(Logger& logger)
            : _logger(logger) {}

        template<typename Ev, typename... Args, 
            typename = std::enable_if_t<
                std::is_invocable_v<typename Ev::event_type, Args&...> &&
                has_type_v<Events, Ev>
            >>
        void call_event(Args&&... args);

        template<typename Ev, typename Listener,
            typename = std::enable_if_t<
                std::is_constructible_v<typename Ev::event_type, Listener> &&
                has_type_v<Events, Ev>
            >>
        void add_listener(Listener listener);

    private:
        Logger& _logger;

        Events _events;
    };
}

#include "event_manager.tpp"

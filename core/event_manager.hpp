#pragma once

#include <tuple>
#include <type_traits>

#include "event.hpp"
#include "logger.hpp"
#include "utils/type_traits.hpp"

namespace Aporia
{
    class Window;

    using EventsContainer = std::tuple<
        WindowCloseEvent,
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
        EndFrameEvent,
        ReloadWindowConfigEvent,
        ReloadTextureConfigEvent,
        ReloadShaderConfigEvent,
        ReloadCameraConfigEvent,
        ReloadAnimationConfigEvent>;

    template<typename T>
    concept EventType = has_type_v<EventsContainer, T>;

    template<typename T, typename Event>
    concept ListenerOf = std::is_constructible_v<typename Event::event_type, T>;

    class EventManager final
    {
    public:
        EventManager(Logger& logger)
            : _logger(logger) {}

        template<EventType Event, typename... Args>
        void call_event(Args&&... args);

        template<EventType Event, ListenerOf<Event> Listener>
        void add_listener(Listener listener);

    private:
        Logger& _logger;

        EventsContainer _events;
    };
}

#include "event_manager.tpp"

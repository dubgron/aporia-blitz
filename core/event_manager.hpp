#pragma once

#include <tuple>
#include <type_traits>

#include "event.hpp"
#include "utils/type_traits.hpp"

namespace Aporia
{
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
        ReloadCameraConfigEvent>;

    template<typename T>
    concept EventType = has_type_v<EventsContainer, T>;

    template<typename T, typename Event>
    concept ListenerOf = std::is_constructible_v<typename Event::event_type, T>;

    class EventManager final
    {
    public:
        EventManager() = default;

        template<EventType Event, typename... Args>
        void call_event(Args&&... args)
        {
            for (auto& func : std::get<Event>(_events).listeners)
            {
                func( std::forward<Args>(args)... );
            }
        }

        template<EventType Event, ListenerOf<Event> Listener>
        void add_listener(Listener listener)
        {
            std::get<Event>(_events).listeners.push_back( std::move(listener) );
        }

    private:
        EventsContainer _events;
    };
}

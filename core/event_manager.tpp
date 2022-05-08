#pragma once

#include <utility>

namespace Aporia
{
    template<EventType Event, typename... Args>
    void EventManager::call_event(Args&&... args)
    {
        for (auto& func : std::get<Event>(_events).listeners)
        {
            func( std::forward<Args>(args)... );
        }
    }

    template<EventType Event, ListenerOf<Event> Listener>
    void EventManager::add_listener(Listener listener)
    {
        std::get<Event>(_events).listeners.push_back( std::move(listener) );
    }
}

#pragma once

#include <utility>

namespace Aporia
{
    template<typename Ev, typename... Args, typename>
    void EventManager::call_event(Args&&... args)
    {
        for (auto& func : std::get<Ev>(_events).listeners)
        {
            func(std::forward<Args>(args)...);
        }
    }

    template<typename Ev, typename Listener, typename>
    void EventManager::add_listener(Listener listener)
    {
        std::get<Ev>(_events).listeners.push_back(std::move(listener));
    }
}

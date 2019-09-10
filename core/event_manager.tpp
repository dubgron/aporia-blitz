#pragma once

namespace Aporia
{
    template<typename T, typename... Ts, typename>
    void EventManager::call_event(Ts&... args)
    {
        for (auto& func : std::get<T>(_events))
        {
            func(args...);
        }
    }
    template<typename T, typename U>
    void EventManager::add_listener(U func)
    {
        std::get<T>(_events).push_back(std::move(func));
    }
}
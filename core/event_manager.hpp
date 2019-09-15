#pragma once

#include <functional>
#include <tuple>
#include <type_traits>

#include "event.hpp"
#include "logger.hpp"
#include "platform.hpp"
#include "window.hpp"

namespace Aporia
{
    class APORIA_API EventManager final
    {
    public:
        EventManager(std::shared_ptr<Logger> logger);

        void listen_for_events(Window& window);

        template<typename T, typename... Ts, 
            typename = std::enable_if_t<std::is_invocable_v<T::event_type, Ts&...>>>
        void call_event(Ts&... args);

        /* TODO: Add std::enable_if check */
        template<typename T, typename U>
        void add_listener(U func);

    private:
        std::tuple<
            WindowCloseEvent,
            WindowResizeEvent> _events = std::make_tuple(WindowCloseEvent(), WindowResizeEvent());

        std::shared_ptr<Logger> _logger;
    };
}

#include "event_manager.tpp"

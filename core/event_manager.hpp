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
            typename = std::enable_if_t<std::is_invocable_v<typename T::event_type, Ts&...>>>
        void call_event(Ts&... args);

        template<typename T, typename U,
            typename = std::enable_if_t<std::is_constructible_v<typename T::event_type, U>>>
        void add_listener(U func);

    private:
        std::tuple<
            WindowCloseEvent,
            WindowResizeEvent> _events = std::make_tuple(WindowCloseEvent(), WindowResizeEvent());

        std::shared_ptr<Logger> _logger;
    };
}

#include "event_manager.tpp"

#pragma once

#include <functional>
#include <memory>

#include "logger.hpp"
#include "window.hpp"

namespace Aporia
{
    template<unsigned int ID, typename... Ts>
    class Event
    {
    public:
        using event_type = typename std::function<void(std::shared_ptr<Logger>, Ts...)>;

        Event() = default;
        Event(event_type) {};

        std::vector<event_type> listeners;

    private:
        unsigned int id = ID;
    };

    template<unsigned int ID, typename... Ts>
    using WindowEvent = Event<ID, Window&, Ts...>;

    using WindowCloseEvent = WindowEvent<0>;
    using WindowResizeEvent = WindowEvent<1, unsigned int, unsigned int>;
}

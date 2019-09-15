#pragma once

#include <functional>

#include "window.hpp"

namespace Aporia
{
    template<unsigned int ID, typename... Ts>
    class Event
    {
    public:
        using event_type = typename std::function<void(Ts...)>;

        Event() = default;
        Event(std::function<void(Ts...)>) {};

        std::vector<std::function<void(Ts...)>> listeners;

    private:
        unsigned int id = ID;
    };

    template<unsigned int ID, typename... Ts>
    using WindowEvent = Event<ID, Window&, Ts...>;

    using WindowCloseEvent = WindowEvent<0>;
    using WindowResizeEvent = WindowEvent<1, unsigned int, unsigned int>;
}

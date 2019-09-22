#pragma once

#include <functional>
#include <memory>

#include "inputs.hpp"
#include "window.hpp"

namespace Aporia
{
    template<unsigned int ID, typename... Ts>
    class Event final
    {
    public:
        using event_type = std::function<void(Ts...)>;

        std::vector<event_type> listeners;

    private:
        unsigned int id = ID;
    };

    template<unsigned int ID, typename... Ts>
    using WindowEvent = Event<ID, Window&, Ts...>;

    using WindowCloseEvent = WindowEvent<0>;
    using WindowResizeEvent = WindowEvent<1, unsigned int, unsigned int>;

    using KeyPressedEvent = Event<2, Keyboard>;
    using KeyReleasedEvent = Event<3, Keyboard>;

    using ButtonPressedEvent = Event<4, Mouse>;
    using ButtonReleasedEvent = Event<5, Mouse>;

    using BeginProcessingWindowEvents = Event<6>;
    using EndProcessingWindowEvents = Event<7>;
}

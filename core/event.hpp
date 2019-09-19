#pragma once

#include <functional>
#include <memory>

#include "inputs.hpp"
#include "logger.hpp"
#include "window.hpp"

namespace Aporia
{
    template<unsigned int ID, typename... Ts>
    class Event
    {
    public:
        using event_type = std::function<void(std::shared_ptr<Logger>, Ts...)>;

        std::vector<event_type> listeners;

    private:
        unsigned int id = ID;
    };

    template<unsigned int ID, typename... Ts>
    using WindowEvent = Event<ID, Window&, Ts...>;

    using WindowCloseEvent = WindowEvent<0>;
    using WindowResizeEvent = WindowEvent<1, unsigned int, unsigned int>;

    using KeyPressedEvent = Event<2, Keyboard, bool, bool, bool, bool>;
    using KeyReleasedEvent = Event<3, Keyboard, bool, bool, bool, bool>;

    using ButtonPressedEvent = Event<4, Mouse, int, int>;
    using ButtonReleasedEvent = Event<5, Mouse, int, int>;
}

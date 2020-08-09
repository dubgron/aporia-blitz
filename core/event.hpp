#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include <glm/vec2.hpp>

#include "inputs/all_inputs.hpp"

namespace Aporia
{
    class Window;

    template<unsigned int ID, typename... Ts>
    class Event final
    {
    public:
        using event_type = std::function<void(Ts...)>;

        std::vector<event_type> listeners;

    private:
        uint32_t id = ID;
    };

    template<unsigned int ID, typename... Ts>
    using WindowEvent = Event<ID, Window&, Ts...>;

    using WindowCloseEvent = WindowEvent<0>;
    using WindowResizeEvent = WindowEvent<1, uint32_t, uint32_t>;

    using KeyPressedEvent = Event<2, Keyboard>;
    using KeyReleasedEvent = Event<3, Keyboard>;

    using ButtonPressedEvent = Event<4, Mouse>;
    using ButtonReleasedEvent = Event<5, Mouse>;

    using MouseWheelScrollEvent = Event<6, MouseWheel, float>;

    using MouseMoveEvent = Event<7, glm::vec2>;

    using BeginProcessingWindowEvents = Event<8>;
    using EndProcessingWindowEvents = Event<9>;
}

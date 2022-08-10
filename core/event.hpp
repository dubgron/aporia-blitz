#pragma once

#include <functional>
#include <utility>
#include <vector>

#include <glm/vec2.hpp>

#include "inputs/all_inputs.hpp"

namespace Aporia
{
    class Window;

    template<size_t ID, typename... Ts>
    struct Event final
    {
        using event_type = std::function<void(Ts...)>;

        std::vector<event_type> listeners;
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

    using BeginFrameEvent = Event<10>;
    using EndFrameEvent = Event<11>;

    using ReloadWindowConfigEvent = Event<12>;
    using ReloadTextureConfigEvent = Event<13>;
    using ReloadShaderConfigEvent = Event<14>;
    using ReloadCameraConfigEvent = Event<15>;
    using ReloadAnimationConfigEvent = Event<16>;
}

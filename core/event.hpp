#pragma once

#include <functional>
#include <utility>
#include <vector>

#include "aporia_types.hpp"
#include "inputs/keyboard.hpp"
#include "inputs/mouse.hpp"

namespace Aporia
{
    class Window;

    template<u64 ID, typename... Ts>
    struct Event final
    {
        using event_type = std::function<void(Ts...)>;

        std::vector<event_type> listeners;
    };

    template<u64 ID, typename... Ts>
    using WindowEvent = Event<ID, Window&, Ts...>;

    using WindowCloseEvent = WindowEvent<0>;
    using WindowResizeEvent = WindowEvent<1, u32, u32>;

    using KeyPressedEvent = Event<2, Keyboard>;
    using KeyReleasedEvent = Event<3, Keyboard>;

    using ButtonPressedEvent = Event<4, Mouse>;
    using ButtonReleasedEvent = Event<5, Mouse>;

    using MouseWheelScrollEvent = Event<6, MouseWheel, f32>;

    using MouseMoveEvent = Event<7, v2>;

    using BeginProcessingWindowEvents = Event<8>;
    using EndProcessingWindowEvents = Event<9>;

    using BeginFrameEvent = Event<10>;
    using EndFrameEvent = Event<11>;

    using ReloadWindowConfigEvent = Event<12>;
    using ReloadTextureConfigEvent = Event<13>;
    using ReloadShaderConfigEvent = Event<14>;
    using ReloadCameraConfigEvent = Event<15>;
}

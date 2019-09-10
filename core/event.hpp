#pragma once

#include <functional>

#include "window.hpp"

namespace Aporia
{
    template<typename... Ts>
    using Event = std::vector<std::function<void(Ts...)>>;

    template<typename... Ts>
    using WindowEvent = Event<Window&, Ts...>;

    using WindowCloseEvent = WindowEvent<>;
    using WindowResizeEvent = WindowEvent<unsigned int, unsigned int>;
}

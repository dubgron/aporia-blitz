#pragma once

#include <optional>
#include <string>

#include "aporia_types.hpp"

namespace Aporia
{
    struct WindowConfig final
    {
        std::string title;

        u32 width = 0;
        u32 height = 0;

        bool vsync = true;

        std::optional<v2_i32> position = std::nullopt;
    };
}

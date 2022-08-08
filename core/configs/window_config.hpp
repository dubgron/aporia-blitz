#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <glm/vec2.hpp>

namespace Aporia
{
    struct WindowConfig final
    {
        std::string title;

        uint32_t width = 0;
        uint32_t height = 0;

        bool vsync = true;

        std::optional<glm::ivec2> position = std::nullopt;
    };
}

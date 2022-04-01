#pragma once

#include <string>
#include <optional>

#include <glm/vec2.hpp>

namespace Aporia
{
    struct WindowConfig final
    {
        std::string title;
        unsigned int width = 0u;
        unsigned int height = 0u;
        bool vsync = true;
        std::optional<glm::ivec2> position = std::nullopt;
    };
}

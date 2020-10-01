#pragma once

#include <string>

#include <glm/vec2.hpp>

namespace Aporia
{
    struct WindowConfig final
    {
        std::string title;
        glm::ivec2 position{ 0 };
        unsigned int width = 0u;
        unsigned int height = 0u;
        bool vsync = true;
    };
}

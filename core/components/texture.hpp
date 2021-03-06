#pragma once

#include <cstdint>
#include <memory>

#include <glm/vec2.hpp>

namespace Aporia
{
    struct Texture final
    {
        glm::vec2 u{ 0.0f };
        glm::vec2 v{ 0.0f };

        struct Origin final
        {
            uint32_t id = 0u;
            int32_t width = 0;
            int32_t height = 0;
            int32_t channels = 0;
        } origin;
    };
}

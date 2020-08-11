#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Aporia
{
    struct Transform2D final
    {
        glm::vec3 position{ 0.0f };

        glm::vec2 origin{ 0.0f };
        float rotation = 0.0f;

        glm::vec2 scale{ 1.0f };
    };
}

#pragma once

#include <utility>

#include <glm/vec3.hpp>

#include "entity.hpp"
#include "components/color.hpp"
#include "components/linear2d.hpp"
#include "components/transform2d.hpp"

namespace Aporia
{
    struct Line2D final : public Entity<Transform2D, Linear2D, Color>
    {
        Line2D() = default;

        Line2D(glm::vec3 p_1, glm::vec3 p_2, Color color = Colors::Black)
            : Entity(Transform2D{ p_1, (p_2 - p_1) / 2.0f }, Linear2D{ p_2 - p_1 }, Color{ color }) {}
    };
}

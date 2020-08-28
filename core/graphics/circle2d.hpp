#pragma once

#include <glm/vec3.hpp>

#include "entity.hpp"
#include "components/circular.hpp"
#include "components/color.hpp"
#include "components/transform2d.hpp"

namespace Aporia
{
    struct Circle2D final : public Entity<Transform2D, Circular, Color>
    {
        Circle2D() = default;

        Circle2D(glm::vec3 position, float radius, Color color = Colors::Black)
            : Entity(Transform2D{ position }, Circular{ radius }, Color{ color }) {}

        Circle2D(float x, float y, float radius, Color color = Colors::Black)
            : Circle2D(glm::vec3{ x, y, 0.0f }, radius, color) {}
    };
}

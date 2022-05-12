#pragma once

#include <glm/vec3.hpp>

#include "entity.hpp"
#include "components/color.hpp"
#include "components/rectangular.hpp"
#include "components/transform2d.hpp"

namespace Aporia
{
    struct Rectangle2D final : public Entity<Transform2D, Rectangular, Color>
    {
        Rectangle2D() = default;

        Rectangle2D(glm::vec3 position, float width, float height, Color color = Colors::White)
            : Entity(Transform2D{ position }, Rectangular{ width, height }, Color{ color }) {}

        Rectangle2D(float x, float y, float width, float height, Color color = Colors::White)
            : Rectangle2D(glm::vec3{ x, y, 0.0f }, width, height, color) {}
    };
}

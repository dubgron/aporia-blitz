#pragma once

#include <cmath>

#include <glm/ext/matrix_transform.hpp>

#include "aporia_types.hpp"
#include "components/transform2d.hpp"

#define min(value, bound)                     ((value) > (bound) ? (bound) : (value))
#define max(value, bound)                     ((value) < (bound) ? (bound) : (value))
#define clamp(value, min_bound, max_bound)    min(max(value, min_bound), max_bound)

namespace Aporia
{
    inline m4 to_mat4(const Transform2D& transform)
    {
        /**
         *  Precalculated following lines:
         *
         *  result = glm::translate(glm::mat4{ 1.f }, transform.position + glm::vec3{ transform.origin, 0.f });
         *  result = glm::rotate(result, transform.rotation, glm::vec3{ 0.f, 0.f, 1.f });
         *  result = glm::scale(result, glm::vec3{ transform.scale, 1.f });
         *  result = glm::translate(result, glm::vec3{ a-transform.origin, 0.f });
         *
         */
        const v2 sin_scaled{ transform.scale * std::sin(transform.rotation) };
        const v2 cos_scaled{ transform.scale * std::cos(transform.rotation) };

        const f32 x_translate = transform.position.x + transform.origin.x - (transform.origin.x * cos_scaled.x - transform.origin.y * sin_scaled.y);
        const f32 y_translate = transform.position.y + transform.origin.y - (transform.origin.x * sin_scaled.x + transform.origin.y * cos_scaled.y);
        const f32 z_translate = transform.position.z;

        return m4{
            cos_scaled.x,   sin_scaled.x,   0.f,            0.f,
            -sin_scaled.y,  cos_scaled.y,   0.f,            0.f,
            0.f,            0.f,            1.f,            0.f,
            x_translate,    y_translate,    z_translate,    1.f };
    }

    inline Transform2D operator*(const Transform2D& transform1, const Transform2D& transform2)
    {
        return Transform2D{
            .position = transform1.position + transform2.position,
            .origin = transform1.origin + transform2.origin,
            .rotation = transform1.rotation + transform2.rotation,
            .scale = transform1.scale * transform2.scale };
    }
}

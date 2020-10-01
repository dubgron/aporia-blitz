#pragma once

#include <cmath>

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "components/transform2d.hpp"

namespace Aporia
{
    inline glm::mat4 to_mat4(const Transform2D& transform)
    {
        glm::mat4 result{ 1.0f };
        float sin = std::sin(transform.rotation);
        float cos = std::cos(transform.rotation);

        glm::vec2 scale_origin = transform.scale * transform.origin;
        glm::vec2 position_origin = transform.position + glm::vec3{ transform.origin, 0.0f };

        float t_x = -scale_origin.x * cos + scale_origin.y * sin + position_origin.x;
        float t_y = -scale_origin.x * sin - scale_origin.y * cos + position_origin.y;

       /**
         *  Precalculated following lines:
         *
         *  result = glm::translate(glm::mat4{ 1.0f }, transform.position + glm::vec3{ transform.origin, 0.0f });
         *  result = glm::rotate(result, transform.rotation, glm::vec3{ 0.0f, 0.0f, 1.0f });
         *  result = glm::scale(result, glm::vec3{ transform.scale, 1.0f });
         *  result = glm::translate(result, glm::vec3{ -transform.origin, 0.0f });
         *
         */
        result[0] = transform.scale.x * glm::vec4{ cos, sin, 0, 0 };
        result[1] = transform.scale.y * glm::vec4{ -sin, cos, 0, 0 };
        result[3] = glm::vec4{ t_x, t_y, transform.position.z, 1.0f };

        return result;
    }
}

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
        result = glm::translate(result, transform.position + glm::vec3{ transform.origin, 0.0f });
        result = glm::rotate(result, transform.rotation, glm::vec3{ 0.0f, 0.0f, 1.0f });
        result = glm::scale(result, glm::vec3{ transform.scale, 1.0f });
        result = glm::translate(result, glm::vec3{ -transform.origin, 0.0f });

        return result;
    }
}

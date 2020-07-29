#pragma once

#include <cmath>

#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Vector2.hpp>

#include "components/transform2d.hpp"

namespace Aporia
{
    inline sf::Vector2f rotate(sf::Vector2f vector, double angle)
    {
        double sin = std::sin(angle);
        double cos = std::cos(angle);

        return sf::Vector2f(vector.x * cos - vector.y * sin, vector.x * sin + vector.y * cos);
    }

    inline sf::Vector2f operator*(const Transform2D& transform, const sf::Vector2f& vector)
    {
        return rotate(vector - transform.origin, transform.rotation) + transform.origin + transform.position;
    }

    inline Transform2D operator*(const Transform2D& transform_1, const Transform2D& transform_2)
    {
        sf::Transform transform;
        transform.translate(transform_1.position);
        transform.rotate(transform_1.rotation * 180.0f / M_PI);
        transform.translate(transform_2.position);
        transform.rotate(transform_2.rotation * 180.0f / M_PI, transform_2.origin);

        const float* matrix = transform.getMatrix();

        Transform2D result;
        result.position = sf::Vector2f(matrix[12], matrix[13]);
        result.rotation = std::atan2(matrix[1], matrix[0]);

        return result;
    }

    inline Transform2D& operator*=(Transform2D& transform_1, const Transform2D& transform_2)
    {
        transform_1 = transform_1 * transform_2;

        return transform_1;
    }
}

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

    inline sf::Vector2f scale(sf::Vector2f vector, sf::Vector2f scale)
    {
        return sf::Vector2f(vector.x * scale.x, vector.y * scale.y);
    }

    inline sf::Vector2f operator*(const Transform2D& transform, const sf::Vector2f& vector)
    {
        return rotate(scale(vector - transform.origin, transform.scale), transform.rotation) + transform.origin + transform.position;
    }

    inline Transform2D operator*(const Transform2D& transform_1, const Transform2D& transform_2)
    {
        sf::Transform transform;
        transform.translate(transform_1.position);
        transform.rotate(transform_1.rotation * 180.0f / M_PI);
        transform.scale(transform_1.scale);
        transform.translate(transform_2.position + transform_2.origin);
        transform.rotate(transform_2.rotation * 180.0f / M_PI);
        transform.scale(transform_2.scale);
        transform.translate(-transform_2.origin);

        const float* matrix = transform.getMatrix();

        Transform2D result;
        result.position = sf::Vector2f(matrix[12], matrix[13]);
        result.scale = { transform_1.scale.x * transform_2.scale.x, transform_1.scale.y * transform_2.scale.y };
        result.rotation = std::atan2(matrix[1] / result.scale.x, matrix[0] / result.scale.x);

        return result;
    }

    inline Transform2D& operator*=(Transform2D& transform_1, const Transform2D& transform_2)
    {
        transform_1 = transform_1 * transform_2;

        return transform_1;
    }
}

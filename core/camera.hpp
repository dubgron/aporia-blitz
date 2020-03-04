#pragma once

#include <SFML/Graphics/Transform.hpp>

#include "platform.hpp"

namespace Aporia
{
    class APORIA_API Camera final
    {
        using Matrix3 = sf::Transform;

    public:
        Camera(float left, float right, float bottom, float top);

        void set_projection_matrix(float left, float right, float bottom, float top);

        void set_position(sf::Vector2f pos);
        void move(const sf::Vector2f& vec);
        const sf::Vector2f& get_position() const;

        void set_angle(float angle);
        void rotate(float angle);

        void set_zoom(float zoom);
        void zoom(float zoom);

        Matrix3 get_view_projection_matrix() const;
        Matrix3 get_rotation() const;

    private:
        Matrix3 _projection_matrix;

        sf::Vector2f _position = { 0.0f, 0.0f };
        float _angle = 0.0f;
        float _zoom = 1.0f;
    };
}

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
        void set_view_matrix(const sf::Vector2f& position, float rotation);

        void set_position(sf::Vector2f pos);
        void move(const sf::Vector2f& vec);
        const sf::Vector2f& get_position() const;

        void set_rotation(float rotation);
        void rotate(float rotation);
        float get_rotation() const;

        void set_zoom(float zoom);
        void zoom(float zoom);
        float get_zoom() const;

        const Matrix3& get_projection_matrix() const;
        const Matrix3& get_view_matrix() const;
        const Matrix3& get_view_projection_matrix() const;

    private:
        Matrix3 _projection_matrix;
        Matrix3 _view_matrix;
        Matrix3 _view_projection_matrix;

        float _left, _right, _bottom, _top;
        sf::Vector2f _position = { 0.0f, 0.0f };
        float _rotation = 0.0f;
        float _zoom = 1.0f;
    };
}

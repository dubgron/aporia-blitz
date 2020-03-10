#include "camera.hpp"

#include <cmath>

namespace Aporia
{
    Camera::Camera(float left, float right, float bottom, float top)
        : _left(left), _right(right), _bottom(bottom), _top(top)
    {
        set_projection_matrix(_left, _right, _bottom, _top);
    }

    void Camera::set_projection_matrix(float left, float right, float bottom, float top)
    {
        _projection_matrix = sf::Transform(2 / (right - left), 0, -(right + left) / (right - left),
                                           0, 2 / (top - bottom), -(top + bottom) / (top - bottom),
                                           0, 0, 1);

        _view_projection_matrix = _projection_matrix * _view_matrix;
    }

    void Camera::set_view_matrix(const sf::Vector2f& position, float rotation)
    {
        float radians = rotation * M_PI / 180.0f;
        float sin = std::sinf(radians);
        float cos = std::cosf(radians);

        _view_matrix = sf::Transform(cos, -sin, position.x,
                                     sin, cos, position.y,
                                     0, 0, 1).getInverse();

        _view_projection_matrix = _projection_matrix * _view_matrix;
    }

    void Camera::set_position(sf::Vector2f pos)
    {
        _position = std::move(pos);
        set_view_matrix(_position, _rotation);
    }

    void Camera::move(const sf::Vector2f& vec)
    {
        set_position(_position + vec);
    }

    inline const sf::Vector2f& Camera::get_position() const
    {
        return _position;
    }

     void Camera::set_rotation(float rotation)
    {
        _rotation = rotation;
        set_view_matrix(_position, _rotation);
    }

    void Camera::rotate(float rotation)
    {
        set_rotation(_rotation + rotation);
    }

    void Camera::set_zoom(float zoom)
    {
        _zoom = zoom;
        set_projection_matrix(_left * _zoom, _right * _zoom, _bottom * _zoom, _top * _zoom);
    }

    void Camera::zoom(float zoom)
    {
        set_zoom(_zoom + zoom);
    }

    const Camera::Matrix3& Camera::get_projection_matrix() const
    {
        return _projection_matrix;
    }

    const Camera::Matrix3& Camera::get_view_matrix() const
    {
        return _view_matrix;
    }

    const Camera::Matrix3& Camera::get_view_projection_matrix() const
    {
        return _view_projection_matrix;
    }
}

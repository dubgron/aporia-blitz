#include "camera.hpp"

namespace Aporia
{
    Camera::Camera(float left, float right, float bottom, float top)
    {
        set_projection_matrix(left, right, bottom, top);
    }

    void Camera::set_projection_matrix(float left, float right, float bottom, float top)
    {
        _projection_matrix = sf::Transform(2 / (right - left), 0, -(right + left) / (right - left),
                                           0, -2 / (top - bottom), -(top + bottom) / (top - bottom),
                                           0, 0, 1);
    }

    void Camera::set_position(sf::Vector2f pos)
    {
        _position = std::move(pos);
    }

    void Camera::move(const sf::Vector2f& vec)
    {
        _position += vec;
    }

    const sf::Vector2f& Camera::get_position() const
    {
        return _position;
    }

    void Camera::set_angle(float angle)
    {
        _angle = angle;
    }

    void Camera::rotate(float angle)
    {
        _angle += angle;
    }

    void Camera::set_zoom(float zoom)
    {
        _zoom = zoom;
    }

    void Camera::zoom(float zoom)
    {
        _zoom += zoom;
    }

    Camera::Matrix3 Camera::get_view_projection_matrix() const
    {
        Matrix3 id;
        Matrix3 view = id.translate(_position).rotate(_angle).scale(_zoom, _zoom).getInverse();

        return _projection_matrix * view;
    }

    Camera::Matrix3 Camera::get_rotation() const
    {
        Matrix3 id;
        return id.rotate(_angle).scale(_zoom, _zoom);
    }
}

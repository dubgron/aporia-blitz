#include "camera.hpp"

#include <cmath>

#include <glm/gtx/transform.hpp>

namespace Aporia
{
    Camera::Camera(float left, float right, float bottom, float top)
        : _left(left), _right(right), _bottom(bottom), _top(top)
    {
        set_projection_matrix(_left, _right, _bottom, _top);
    }

    void Camera::set_projection_matrix(float left, float right, float bottom, float top)
    {
        _projection_matrix = glm::ortho(left, right, bottom, top);

        _view_projection_matrix = _projection_matrix * _view_matrix;
    }

    void Camera::set_view_matrix(const glm::vec2& position, float rotation)
    {
        float x = position.x;
        float y = position.y;
        float sin = std::sinf(rotation);
        float cos = std::cosf(rotation);

        /**
         *  Precalculated following lines:
         *
         *  _view_matrix = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ position, 0.0f });
         *  _view_matrix = glm::rotate(_view_matrix, rotation, glm::vec3{ 0.0f, 0.0f, 1.0f });
         *  _view_matrix = glm::inverse(_view_matrix);
         *
         */
        _view_matrix[0] = { cos, -sin, 0, 0 };
        _view_matrix[1] = { sin, cos, 0, 0 };
        _view_matrix[2] = { 0, 0, 1, 0 };
        _view_matrix[3] = { -x * cos - y * sin, x * sin - y * cos, 0, 1 };

        _view_projection_matrix = _projection_matrix * _view_matrix;
    }

    void Camera::set_frustum(float left, float right, float bottom, float top)
    {
        _left = left;
        _right = right;
        _bottom = bottom;
        _top = top;

        set_projection_matrix(_left * _zoom, _right * _zoom, _bottom * _zoom, _top * _zoom);
    }

    void Camera::set_position(glm::vec2 pos)
    {
        _position = std::move(pos);
        set_view_matrix(_position, _rotation);
    }

    void Camera::move(const glm::vec2& vec)
    {
        set_position(_position + vec);
    }

    const glm::vec2& Camera::get_position() const
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

    float Camera::get_rotation() const
    {
        return _rotation;
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

    float Camera::get_zoom() const
    {
        return _zoom;
    }

    const glm::mat4& Camera::get_projection_matrix() const
    {
        return _projection_matrix;
    }

    const glm::mat4& Camera::get_view_matrix() const
    {
        return _view_matrix;
    }

    const glm::mat4& Camera::get_view_projection_matrix() const
    {
        return _view_projection_matrix;
    }
}

#include "camera.hpp"

#include <cmath>

#include <glm/gtx/transform.hpp>

namespace Aporia
{
    Camera::Camera(float fov, float aspect_ratio)
        : _projection{ .fov = fov, .aspect_ratio = aspect_ratio }
    {
    }

    const glm::mat4& Camera::get_view_projection_matrix()
    {
        if (is_marked_dirty(DIRTYFLAG_VIEW))
        {
            recalculate_view();
        }

        if (is_marked_dirty(DIRTYFLAG_PROJECTION))
        {
            recalculate_projection();
        }

        if (_dirty_flags)
        {
            _vp_matrix = _projection.matrix * _view.matrix;
            _dirty_flags = 0;
        }

        return _vp_matrix;
    }

    void Camera::set_position(glm::vec2 new_position)
    {
        mark_as_dirty(DIRTYFLAG_VIEW);
        _view.position = new_position;
    }

    void Camera::move(const glm::vec2& vector)
    {
        set_position(_view.position + vector);
    }

    const glm::vec2& Camera::get_position() const
    {
        return _view.position;
    }

    void Camera::set_rotation(float new_rotation)
    {
        mark_as_dirty(DIRTYFLAG_VIEW);
        _view.rotation = new_rotation;
    }

    void Camera::rotate(float rotation)
    {
        set_rotation(_view.rotation + rotation);
    }

    float Camera::get_rotation() const
    {
        return _view.rotation;
    }

    const glm::vec2& Camera::get_up_vector() const
    {
        return _view.up_vector;
    }

    const glm::vec2& Camera::get_right_vector() const
    {
        return _view.right_vector;
    }

    void Camera::set_fov(float new_fov)
    {
        mark_as_dirty(DIRTYFLAG_PROJECTION);
        _projection.fov = new_fov;
    }

    float Camera::get_fov() const
    {
        return _projection.fov;
    }

    void Camera::set_aspect_ratio(float new_aspect_ratio)
    {
        mark_as_dirty(DIRTYFLAG_PROJECTION);
        _projection.aspect_ratio = new_aspect_ratio;
    }

    float Camera::get_aspect_ratio() const
    {
        return _projection.aspect_ratio;
    }

    void Camera::set_zoom(float new_zoom)
    {
        mark_as_dirty(DIRTYFLAG_PROJECTION);
        _projection.zoom = new_zoom;
    }

    void Camera::zoom(float zoom)
    {
        set_zoom(_projection.zoom + zoom);
    }

    float Camera::get_zoom() const
    {
        return _projection.zoom;
    }

    void Camera::recalculate_view()
    {
        const float x = _view.position.x;
        const float y = _view.position.y;

        const float sin = std::sin(_view.rotation);
        const float cos = std::cos(_view.rotation);

        _view.right_vector.x = cos;
        _view.right_vector.y = sin;

        _view.up_vector.x = -sin;
        _view.up_vector.y = cos;

        /**
         *  Precalculated the following lines:
         *
         *  _view.matrix = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ position, 0.0f });
         *  _view.matrix = glm::rotate(_view.matrix, rotation, glm::vec3{ 0.0f, 0.0f, 1.0f });
         *  _view.matrix = glm::inverse(_view.matrix);
         *
         */
        _view.matrix[0][0] = cos;
        _view.matrix[0][1] = -sin;
        _view.matrix[1][0] = sin;
        _view.matrix[1][1] = cos;
        _view.matrix[3][0] = -x * cos - y * sin;
        _view.matrix[3][1] = x * sin - y * cos;
    }

    void Camera::recalculate_projection()
    {
        const float half_height = _projection.fov * _projection.zoom;
        const float half_width = half_height * _projection.aspect_ratio;

        _projection.matrix = glm::ortho(-half_width, half_width, -half_height, half_height, -1.0f, 1.0f);
    }
}

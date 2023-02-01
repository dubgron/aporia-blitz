#include "camera.hpp"

#include <cmath>

#include <glm/gtx/transform.hpp>

namespace Aporia
{
    Camera::Camera(f32 fov, f32 aspect_ratio)
        : _projection{ .fov = fov, .aspect_ratio = aspect_ratio } {}

    const glm::mat4& Camera::get_view_projection_matrix() const
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

    void Camera::set_position(v2 new_position)
    {
        mark_as_dirty(DIRTYFLAG_VIEW);
        _view.position = new_position;
    }

    void Camera::move(const v2& vector)
    {
        set_position(_view.position + vector);
    }

    const v2& Camera::get_position() const
    {
        return _view.position;
    }

    void Camera::set_rotation(f32 new_rotation)
    {
        mark_as_dirty(DIRTYFLAG_VIEW);
        _view.rotation = new_rotation;
    }

    void Camera::rotate(f32 rotation)
    {
        set_rotation(_view.rotation + rotation);
    }

    f32 Camera::get_rotation() const
    {
        return _view.rotation;
    }

    const v2& Camera::get_up_vector() const
    {
        return _view.up_vector;
    }

    const v2& Camera::get_right_vector() const
    {
        return _view.right_vector;
    }

    void Camera::set_fov(f32 new_fov)
    {
        mark_as_dirty(DIRTYFLAG_PROJECTION);
        _projection.fov = new_fov;
    }

    f32 Camera::get_fov() const
    {
        return _projection.fov;
    }

    void Camera::set_aspect_ratio(f32 new_aspect_ratio)
    {
        mark_as_dirty(DIRTYFLAG_PROJECTION);
        _projection.aspect_ratio = new_aspect_ratio;
    }

    f32 Camera::get_aspect_ratio() const
    {
        return _projection.aspect_ratio;
    }

    void Camera::set_zoom(f32 new_zoom)
    {
        mark_as_dirty(DIRTYFLAG_PROJECTION);
        _projection.zoom = new_zoom;
    }

    void Camera::zoom(f32 zoom)
    {
        set_zoom(_projection.zoom + zoom);
    }

    f32 Camera::get_zoom() const
    {
        return _projection.zoom;
    }

    void Camera::recalculate_view() const
    {
        const f32 x = _view.position.x;
        const f32 y = _view.position.y;

        const f32 sin = std::sin(_view.rotation);
        const f32 cos = std::cos(_view.rotation);

        _view.right_vector.x = cos;
        _view.right_vector.y = sin;

        _view.up_vector.x = -sin;
        _view.up_vector.y = cos;

        /**
         *  Precalculated the following lines:
         *
         *  _view.matrix = glm::translate(glm::mat4{ 1.f }, glm::vec3{ position, 0.f });
         *  _view.matrix = glm::rotate(_view.matrix, rotation, glm::vec3{ 0.f, 0.f, 1.f });
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

    void Camera::recalculate_projection() const
    {
        const f32 half_height = _projection.fov * _projection.zoom;
        const f32 half_width = half_height * _projection.aspect_ratio;

        _projection.matrix = glm::ortho(-half_width, half_width, -half_height, half_height, -1.f, 1.f);
    }
}

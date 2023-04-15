#include "aporia_camera.hpp"

#include "aporia_debug.hpp"
#include "aporia_config.hpp"
#include "aporia_inputs.hpp"

namespace Aporia
{
    static void recalculate_view(Camera& camera)
    {
        const f32 x = camera.view.position.x;
        const f32 y = camera.view.position.y;

        const f32 sin = std::sin(camera.view.rotation);
        const f32 cos = std::cos(camera.view.rotation);

        camera.view.right_vector.x = cos;
        camera.view.right_vector.y = sin;

        camera.view.up_vector.x = -sin;
        camera.view.up_vector.y = cos;

        // @NOTE(dubgron): Precalculated the following lines:
        //     camera.view.matrix = glm::translate(glm::mat4{ 1.f }, glm::vec3{ position, 0.f });
        //     camera.view.matrix = glm::rotate(camera.view.matrix, rotation, glm::vec3{ 0.f, 0.f, 1.f });
        //     camera.view.matrix = glm::inverse(camera.view.matrix);
        camera.view.matrix[0][0] = cos;
        camera.view.matrix[0][1] = -sin;
        camera.view.matrix[1][0] = sin;
        camera.view.matrix[1][1] = cos;
        camera.view.matrix[3][0] = -x * cos - y * sin;
        camera.view.matrix[3][1] = x * sin - y * cos;
    }

    static void recalculate_projection(Camera& camera)
    {
        const f32 half_height = camera.projection.fov * camera.projection.zoom;
        const f32 half_width = half_height * camera.projection.aspect_ratio;

        camera.projection.matrix = glm::ortho(-half_width, half_width, -half_height, half_height, -1.f, 1.f);
    }

    void Camera::init()
    {
        projection = CameraProjection{ .fov = camera_config.fov, .aspect_ratio = camera_config.aspect_ratio };
    }

    const m4& Camera::calculate_view_projection_matrix()
    {
        if (is_marked_dirty(CameraDirtyFlag_View))
        {
            recalculate_view(*this);
        }

        if (is_marked_dirty(CameraDirtyFlag_Projection))
        {
            recalculate_projection(*this);
        }

        if (dirty_flags)
        {
            view_projection_matrix = projection.matrix * view.matrix;
            dirty_flags = 0;
        }

        return view_projection_matrix;
    }

    void Camera::set_position(v2 new_position)
    {
        mark_as_dirty(CameraDirtyFlag_View);
        view.position = new_position;
    }

    void Camera::move(v2 vector)
    {
        set_position(view.position + vector);
    }

    void Camera::set_rotation(f32 new_rotation)
    {
        mark_as_dirty(CameraDirtyFlag_View);
        view.rotation = new_rotation;
    }

    void Camera::rotate(f32 rotation)
    {
        set_rotation(view.rotation + rotation);
    }

    void Camera::set_fov(f32 new_fov)
    {
        mark_as_dirty(CameraDirtyFlag_Projection);
        projection.fov = new_fov;
    }

    void Camera::set_aspect_ratio(f32 new_aspect_ratio)
    {
        mark_as_dirty(CameraDirtyFlag_Projection);
        projection.aspect_ratio = new_aspect_ratio;
    }

    void Camera::set_zoom(f32 new_zoom)
    {
        mark_as_dirty(CameraDirtyFlag_Projection);
        projection.zoom = new_zoom;
    }

    void Camera::zoom(f32 zoom)
    {
        set_zoom(projection.zoom + zoom);
    }

    void Camera::control_movement(f32 delta_time)
    {
        const f32 movement_speed = camera_config.movement_speed * delta_time;
        v2 movement{ 0.f };

        if (has_been_held(camera_config.movement_key_up))
        {
            movement.y += movement_speed;
        }

        if (has_been_held(camera_config.movement_key_down))
        {
            movement.y -= movement_speed;
        }

        if (has_been_held(camera_config.movement_key_left))
        {
            movement.x -= movement_speed;
        }

        if (has_been_held(camera_config.movement_key_right))
        {
            movement.x += movement_speed;
        }

        if (movement.x || movement.y)
        {
            const v2 to_move = view.right_vector * movement.x + view.up_vector * movement.y;
            move(to_move);
        }
    }

    void Camera::control_rotation(f32 delta_time)
    {
        const f32 rotation_speed = camera_config.rotation_speed * delta_time;
        f32 rotation = 0.f;

        if (has_been_held(camera_config.rotation_key_left))
        {
            rotation += rotation_speed;
        }

        if (has_been_held(camera_config.rotation_key_right))
        {
            rotation -= rotation_speed;
        }

        if (rotation)
        {
            rotate(rotation);
        }
    }

    void Camera::control_zoom(f32 delta_time)
    {
        const f32 zoom_speed = camera_config.zoom_speed * delta_time;
        f32 zoom = 0.f;

        if (has_been_held(camera_config.zoom_key_in))
        {
            zoom -= zoom_speed;
        }

        if (has_been_held(camera_config.zoom_key_out))
        {
            zoom += zoom_speed;
        }

        if (zoom)
        {
            const f32 new_zoom = std::clamp(projection.zoom + zoom, camera_config.zoom_min, camera_config.zoom_max);
            set_zoom(new_zoom);
        }
    }

    void Camera::follow(v2 to_follow, f32 delta_time)
    {
        const v2 direction{ to_follow - view.position };
        const v2 to_move{ direction * camera_config.movement_speed * delta_time * projection.zoom / projection.fov };
        move(to_move);
    }

    void Camera::on_window_resize(u32 width, u32 height)
    {
        APORIA_ASSERT(width > 0 && height > 0);
        const f32 aspect_ratio = static_cast<f32>(width) / static_cast<f32>(height);
        set_aspect_ratio(aspect_ratio);
    }
}

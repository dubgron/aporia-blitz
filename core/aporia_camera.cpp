#include "aporia_camera.hpp"

#include "aporia_debug.hpp"
#include "aporia_config.hpp"
#include "aporia_input.hpp"
#include "aporia_rendering.hpp"
#include "aporia_window.hpp"

#include <glm/gtx/norm.hpp>

Camera active_camera;

static void recalculate_view(Camera* camera)
{
    f32 x = camera->view.position.x;
    f32 y = camera->view.position.y;

    f32 sin = std::sin(camera->view.rotation);
    f32 cos = std::cos(camera->view.rotation);

    camera->view.right_vector.x = cos;
    camera->view.right_vector.y = sin;

    camera->view.up_vector.x = -sin;
    camera->view.up_vector.y = cos;

    // @NOTE(dubgron): Precalculated the following lines:
    //     camera->view.matrix = glm::translate(glm::mat4{ 1.f }, glm::vec3{ position, 0.f });
    //     camera->view.matrix = glm::rotate(camera->view.matrix, rotation, glm::vec3{ 0.f, 0.f, 1.f });
    //     camera->view.matrix = glm::inverse(camera->view.matrix);
    camera->view.matrix[0][0] = cos;
    camera->view.matrix[0][1] = -sin;
    camera->view.matrix[1][0] = sin;
    camera->view.matrix[1][1] = cos;
    camera->view.matrix[3][0] = -x * cos - y * sin;
    camera->view.matrix[3][1] = x * sin - y * cos;
}

static void recalculate_projection(Camera* camera)
{
    f32 half_height = camera->projection.fov * camera->projection.zoom;
    f32 half_width = half_height * camera->projection.aspect_ratio;

    camera->projection.matrix = glm::ortho(-half_width, half_width, -half_height, half_height);
}

const m4& camera_calculate_view_projection_matrix(Camera* camera)
{
    if (camera->dirty_flags & CameraDirtyFlag_View)
    {
        recalculate_view(camera);
    }

    if (camera->dirty_flags & CameraDirtyFlag_Projection)
    {
        recalculate_projection(camera);
    }

    if (camera->dirty_flags)
    {
        camera->view_projection_matrix = camera->projection.matrix * camera->view.matrix;
        camera->dirty_flags = 0;
    }

    return camera->view_projection_matrix;
}

void camera_control_movement(Camera* camera)
{
    if (input_is_held(Key_LShift))
        return;

    static v2 initial_mouse_position, initial_camera_position;

    v2 current_mouse_position = get_mouse_viewport_position();

    InputState right_mouse_button = input_get(Mouse_Right);
    if (input_is_pressed(right_mouse_button))
    {
        initial_mouse_position = current_mouse_position;
        initial_camera_position = camera->view.position;
    }
    else if (input_is_held(right_mouse_button))
    {
        v2 mouse_position_offset = current_mouse_position - initial_mouse_position;
        v2 rotated_mouse_offset = mouse_position_offset.x * camera->view.right_vector + mouse_position_offset.y * camera->view.up_vector;

        camera->view.position = initial_camera_position - rotated_mouse_offset * camera->projection.zoom;
        camera->dirty_flags |= CameraDirtyFlag_View;
    }
}

void camera_control_rotation(Camera* camera)
{
    if (!input_is_held(Key_LShift))
        return;

    static v2 initial_rotation_right, initial_rotation_up;
    static v2 initial_camera_right, initial_camera_up;

    v2 current_mouse_position = get_mouse_viewport_position();
    v2 half_viewport_size{ viewport_width / 2.f, viewport_height / 2.f };

    InputState right_mouse_button = input_get(Mouse_Right);
    if (input_is_pressed(right_mouse_button))
    {
        initial_rotation_right = glm::normalize(current_mouse_position - half_viewport_size);
        initial_rotation_up = v2{ -initial_rotation_right.y, initial_rotation_right.x };

        initial_camera_right = camera->view.right_vector;
        initial_camera_up = camera->view.up_vector;
    }
    else if (input_is_held(right_mouse_button))
    {
        v2 rotation_dir = glm::normalize(current_mouse_position - half_viewport_size);

        v2 projected_dir = glm::dot(initial_rotation_right, rotation_dir) * initial_camera_right
            - glm::dot(initial_rotation_up, rotation_dir) * initial_camera_up;

        camera->view.rotation = std::atan2(projected_dir.y, projected_dir.x);
        camera->dirty_flags |= CameraDirtyFlag_View;
    }
}

void camera_control_zoom(Camera* camera, f32 delta_time)
{
    f32 zoom_dir = -input_get(Mouse_VerticalWheel).end_value;
    if (zoom_dir != 0.f)
    {
        f32 zoom_speed = camera_config.zoom_speed * delta_time;
        f32 zoom = zoom_speed * zoom_dir;

        if (input_is_held(Key_LShift))
            zoom *= 5.f;

        if (input_is_held(Key_LControl))
            zoom /= 5.f;

        camera->projection.zoom = std::clamp(camera->projection.zoom + zoom, camera_config.zoom_min, camera_config.zoom_max);
        camera->dirty_flags |= CameraDirtyFlag_Projection;
    }
}

void camera_follow(Camera* camera, v2 to_follow, f32 delta_time)
{
    v2 direction{ to_follow - camera->view.position };

    f32 dist2 = glm::length2(direction);
    if (dist2 > 0.1f)
    {
        f32 velocity = camera_config.movement_speed * delta_time * camera->projection.zoom / camera->projection.fov;
        camera->view.position += direction * velocity;
        camera->dirty_flags |= CameraDirtyFlag_View;
    }
}

void camera_apply_config(Camera* camera)
{
    camera_adjust_aspect_ratio_to_render_surface(camera);

    if (camera_config.fov > 0.f)
    {
        camera->projection.fov = camera_config.fov;
    }
    else if (rendering_config.custom_game_resolution_height > 0)
    {
        camera->projection.fov = rendering_config.custom_game_resolution_height / 2.f;
    }
    else
    {
        camera->projection.fov = window_config.height / 2.f;
    }
}

void camera_adjust_aspect_ratio_to_render_surface(Camera* camera)
{
    camera->projection.aspect_ratio = (f32)game_render_width / (f32)game_render_height;
    camera->dirty_flags |= CameraDirtyFlag_Projection;
}

#include "aporia_camera.hpp"

#include "aporia_debug.hpp"
#include "aporia_config.hpp"
#include "aporia_input.hpp"
#include "aporia_rendering.hpp"
#include "aporia_window.hpp"

Camera* active_camera = nullptr;

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

const m4& Camera::calculate_view_projection_matrix()
{
    if (is_marked_dirty(CameraDirtyFlag_View))
    {
        recalculate_view(this);
    }

    if (is_marked_dirty(CameraDirtyFlag_Projection))
    {
        recalculate_projection(this);
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
    if (input_is_held(Key_LShift))
        return;

    static v2 initial_mouse_position, initial_camera_position;

    v2 current_mouse_position = get_mouse_viewport_position();

    InputState right_mouse_button = input_get(Mouse_Right);
    if (input_is_pressed(right_mouse_button))
    {
        initial_mouse_position = current_mouse_position;
        initial_camera_position = view.position;
    }
    else if (input_is_held(right_mouse_button))
    {
        v2 mouse_position_offset = current_mouse_position - initial_mouse_position;
        v2 rotated_mouse_offset = mouse_position_offset.x * view.right_vector + mouse_position_offset.y * view.up_vector;

        v2 new_position = initial_camera_position - rotated_mouse_offset * projection.zoom;
        set_position(new_position);
    }
}

void Camera::control_rotation(f32 delta_time)
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

        initial_camera_right = view.right_vector;
        initial_camera_up = view.up_vector;
    }
    else if (input_is_held(right_mouse_button))
    {
        v2 rotation_dir = glm::normalize(current_mouse_position - half_viewport_size);

        v2 projected_dir = glm::dot(initial_rotation_right, rotation_dir) * initial_camera_right
            - glm::dot(initial_rotation_up, rotation_dir) * initial_camera_up;

        f32 new_rotation = std::atan2(projected_dir.y, projected_dir.x);
        set_rotation(new_rotation);
    }
}

void Camera::control_zoom(f32 delta_time)
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

        f32 new_zoom = std::clamp(projection.zoom + zoom, camera_config.zoom_min, camera_config.zoom_max);
        set_zoom(new_zoom);
    }
}

void Camera::follow(v2 to_follow, f32 delta_time)
{
    v2 direction{ to_follow - view.position };
    v2 to_move{ direction * camera_config.movement_speed * delta_time * projection.zoom / projection.fov };
    move(to_move);
}

void Camera::apply_config()
{
    adjust_aspect_ratio_to_render_surface();

    if (camera_config.fov > 0.f)
    {
        projection.fov = camera_config.fov;
    }
    else if (rendering_config.custom_game_resolution_height > 0)
    {
        projection.fov = rendering_config.custom_game_resolution_height / 2.f;
    }
    else
    {
        projection.fov = window_config.height / 2.f;
    }
}

void Camera::adjust_aspect_ratio_to_render_surface()
{
    mark_as_dirty(CameraDirtyFlag_Projection);
    projection.aspect_ratio = (f32)game_render_width / (f32)game_render_height;
}

Camera* create_camera(MemoryArena* arena)
{
    Camera* result = arena_push_uninitialized<Camera>(arena);
    *result = Camera{};
    result->apply_config();

    return result;
}

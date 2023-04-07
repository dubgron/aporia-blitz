#include "camera_controller.hpp"

#include <algorithm>

#include "aporia_config.hpp"
#include "aporia_inputs.hpp"
#include "aporia_window.hpp"
#include "components/color.hpp"

namespace Aporia
{
    void CameraController::init(const CameraConfig& config)
    {
        _config = &config;
        _camera.init(config.fov, config.aspect_ratio);
    }

    void CameraController::control_movement(f32 delta_time)
    {
        const f32 movement_speed = _config->movement_speed * delta_time;
        v2 movement{ 0.f };

        if (is_key_pressed(_config->movement_key_up))
        {
            movement.y += movement_speed;
        }

        if (is_key_pressed(_config->movement_key_down))
        {
            movement.y -= movement_speed;
        }

        if (is_key_pressed(_config->movement_key_left))
        {
            movement.x -= movement_speed;
        }

        if (is_key_pressed(_config->movement_key_right))
        {
            movement.x += movement_speed;
        }

        if (movement.x || movement.y)
        {
            const v2 to_move = _camera.get_right_vector() * movement.x + _camera.get_up_vector() * movement.y;
            _camera.move(to_move);
        }
    }

    void CameraController::control_rotation(f32 delta_time)
    {
        const f32 rotation_speed = _config->rotation_speed * delta_time;
        f32 rotation = 0.f;

        if (is_key_pressed(_config->rotation_key_left))
        {
            rotation += rotation_speed;
        }

        if (is_key_pressed(_config->rotation_key_right))
        {
            rotation -= rotation_speed;
        }

        if (rotation)
        {
            _camera.rotate(rotation);
        }
    }

    void CameraController::control_zoom(f32 delta_time)
    {
        const f32 zoom_speed = _config->zoom_speed * delta_time;
        f32 zoom = 0.f;

        if (is_key_pressed(_config->zoom_key_in))
        {
            zoom -= zoom_speed;
        }

        if (is_key_pressed(_config->zoom_key_out))
        {
            zoom += zoom_speed;
        }

        if (zoom)
        {
            const f32 new_zoom = std::clamp(_camera.get_zoom() + zoom, _config->zoom_min, _config->zoom_max);
            _camera.set_zoom(new_zoom);
        }
    }

    void CameraController::follow(const v2& to_follow, f32 delta_time)
    {
        const v2 direction{ to_follow - _camera.get_position() };
        const v2 to_move{ direction * _config->movement_speed * delta_time * _camera.get_zoom() / _camera.get_fov() };
        _camera.move(to_move);
    }

    Camera& CameraController::get_camera()
    {
        return _camera;
    }

    const Camera& CameraController::get_camera() const
    {
        return _camera;
    }

    const Color& CameraController::get_clear_color() const
    {
        return _config->background_color;
    }

    void CameraController::on_window_resize(u32 width, u32 height)
    {
        const f32 aspect_ratio = static_cast<f32>(width) / static_cast<f32>(height);
        _camera.set_aspect_ratio(aspect_ratio);
    }

    void CameraController::on_config_reload()
    {
        _camera.set_fov(_config->fov);
        _camera.set_aspect_ratio(_config->aspect_ratio);
    }
}

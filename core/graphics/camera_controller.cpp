#include "camera_controller.hpp"

#include <algorithm>
#include <cmath>
#include <functional>

#include "common.hpp"
#include "event_manager.hpp"
#include "input_manager.hpp"
#include "window.hpp"
#include "components/color.hpp"
#include "configs/camera_config.hpp"

namespace Aporia
{
    CameraController::CameraController(Logger& logger, EventManager& event_manager, CameraConfig& config)
        : _logger(logger),
          _camera(config.fov, config.aspect_ratio),
          _config(config)
    {
        using namespace std::placeholders;

        event_manager.add_listener<WindowResizeEvent>(std::bind(&CameraController::_on_resize, this, _1, _2, _3));
        event_manager.add_listener<ReloadCameraConfigEvent>(std::bind(&CameraController::_on_config_reload, this));
    }

    void CameraController::control_movement(const InputManager& input_manager, float delta_time)
    {
        const float movement_speed = _config.movement_speed * delta_time;
        glm::vec2 movement{ 0.0f };

        if (input_manager.is_key_pressed(_config.movement_key_up))
        {
            movement.y += movement_speed;
        }

        if (input_manager.is_key_pressed(_config.movement_key_down))
        {
            movement.y -= movement_speed;
        }

        if (input_manager.is_key_pressed(_config.movement_key_left))
        {
            movement.x -= movement_speed;
        }

        if (input_manager.is_key_pressed(_config.movement_key_right))
        {
            movement.x += movement_speed;
        }

        if (movement.x || movement.y)
        {
            const glm::vec2 to_move = _camera.get_right_vector() * movement.x + _camera.get_up_vector() * movement.y;
            _camera.move(to_move);
        }
    }

    void CameraController::control_rotation(const InputManager& input_manager, float delta_time)
    {
        const float rotation_speed = _config.rotation_speed * delta_time;
        float rotation = 0.0f;

        if (input_manager.is_key_pressed(_config.rotation_key_left))
        {
            rotation += rotation_speed;
        }

        if (input_manager.is_key_pressed(_config.rotation_key_right))
        {
            rotation -= rotation_speed;
        }

        if (rotation)
        {
            _camera.rotate(rotation);
        }
    }

    void CameraController::control_zoom(const InputManager& input_manager, float delta_time)
    {
        const float zoom_speed = _config.zoom_speed * delta_time;
        float zoom = 0.0f;

        if (input_manager.is_key_pressed(_config.zoom_key_in))
        {
            zoom -= zoom_speed;
        }

        if (input_manager.is_key_pressed(_config.zoom_key_out))
        {
            zoom += zoom_speed;
        }

        if (zoom)
        {
            const float new_zoom = std::clamp(_camera.get_zoom() + zoom, _config.zoom_min, _config.zoom_max);
            _camera.set_zoom(new_zoom);
        }
    }

    void CameraController::follow(const glm::vec2& to_follow, float delta_time)
    {
        const glm::vec2 direction{ to_follow - _camera.get_position() };
        const glm::vec2 to_move{ direction * _config.movement_speed * delta_time * _camera.get_zoom() / _camera.get_fov() };
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
        return _config.background_color;
    }

    void CameraController::_on_resize(Window& window, uint32_t width, uint32_t height)
    {
        _config.aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

        _camera.set_aspect_ratio(_config.aspect_ratio);
    }

    void CameraController::_on_config_reload()
    {
        _camera.set_fov(_config.fov);
        _camera.set_aspect_ratio(_config.aspect_ratio);
    }
}

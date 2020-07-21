#include "camera_controller.hpp"

#include <algorithm>

namespace Aporia
{
    CameraController::CameraController(Logger& logger, const CameraConfig& config)
        : _logger(logger),
          _camera(-config.aspect_ratio * config.size, config.aspect_ratio* config.size, -config.size, config.size),
          _config(config)
    {
    }

    void CameraController::control_movement(const InputManager& input_manager, float delta_time)
    {
        float movement_speed = _config.movement_speed * delta_time;
        sf::Vector2f movement;

        if (input_manager.is_key_pressed(_config.movement_key_up))
            movement += sf::Vector2f(0.0f, movement_speed);

        if (input_manager.is_key_pressed(_config.movement_key_down))
            movement += sf::Vector2f(0.0f, -movement_speed);

        if (input_manager.is_key_pressed(_config.movement_key_left))
            movement += sf::Vector2f(-movement_speed, 0.0f);

        if (input_manager.is_key_pressed(_config.movement_key_right))
            movement += sf::Vector2f(movement_speed, 0.0f);

        if (movement.x || movement.y)
            _camera.move(movement);
    }

    void CameraController::control_rotation(const InputManager& input_manager, float delta_time)
    {
        float rotation_speed = _config.rotation_speed * delta_time;
        float rotation = 0.0f;

        if (input_manager.is_key_pressed(_config.rotation_key_left))
            rotation += rotation_speed;

        if (input_manager.is_key_pressed(_config.rotation_key_right))
            rotation -= rotation_speed;

        if (rotation)
            _camera.rotate(rotation);
    }

    void CameraController::control_zoom(const InputManager& input_manager, float delta_time)
    {
        float zoom_speed = _config.zoom_speed * delta_time;
        float zoom = 0.0f;

        if (input_manager.is_key_pressed(_config.zoom_key_in))
            zoom -= zoom_speed;

        if (input_manager.is_key_pressed(_config.zoom_key_out))
            zoom += zoom_speed;

        if (zoom)
            _camera.set_zoom(std::clamp(_camera.get_zoom() + zoom, _config.zoom_min, _config.zoom_max));
    }

    void CameraController::follow(const sf::Vector2f& to_follow, float delta_time)
    {
        sf::Vector2f dir = to_follow - _camera.get_position();
        _camera.move(dir * _config.movement_speed * delta_time * _camera.get_zoom() / _config.size);
    }

    const Camera& CameraController::get_camera() const
    {
        return _camera;
    }
}

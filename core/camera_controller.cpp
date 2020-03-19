#include "camera_controller.hpp"

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

        if (input_manager.is_key_pressed(_config.movement_key_up))
            _camera.move(sf::Vector2f(0.0f, movement_speed));

        if (input_manager.is_key_pressed(_config.movement_key_down))
            _camera.move(sf::Vector2f(0.0f, -movement_speed));

        if (input_manager.is_key_pressed(_config.movement_key_left))
            _camera.move(sf::Vector2f(-movement_speed, 0.0f));

        if (input_manager.is_key_pressed(_config.movement_key_right))
            _camera.move(sf::Vector2f(movement_speed, 0.0f));
    }

    void CameraController::control_rotation(const InputManager& input_manager, float delta_time)
    {
        float rotation_speed = _config.rotation_speed * delta_time;

        if (input_manager.is_key_pressed(_config.rotation_key_left))
            _camera.rotate(rotation_speed);

        if (input_manager.is_key_pressed(_config.rotation_key_right))
            _camera.rotate(-rotation_speed);
    }

    void CameraController::control_zoom(const InputManager& input_manager, float delta_time)
    {
        float zoom_speed = _config.zoom_speed * delta_time;

        if (input_manager.is_key_pressed(_config.zoom_key_in))
        {
            _zoom_level = std::max(_zoom_level - zoom_speed, _config.zoom_min);
            _camera.set_zoom(_zoom_level);
        }

        if (input_manager.is_key_pressed(_config.zoom_key_out))
        {
            _zoom_level = std::min(_zoom_level + zoom_speed, _config.zoom_max);
            _camera.set_zoom(_zoom_level);
        }
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

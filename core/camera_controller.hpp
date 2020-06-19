#pragma once

#include <SFML/Graphics.hpp>

#include "camera.hpp"
#include "configs/camera_config.hpp"
#include "input_manager.hpp"
#include "logger.hpp"
#include "platform.hpp"

namespace Aporia
{
    class APORIA_API CameraController
    {
    public:
        CameraController(Logger& logger, const CameraConfig& config);

        void control_movement(const InputManager& input_manager, float delta_time);
        void control_rotation(const InputManager& input_manager, float delta_time);
        void control_zoom(const InputManager& input_manager, float delta_time);
        void follow(const sf::Vector2f& to_follow, float delta_time);

        const Camera& get_camera() const;

    private:
        Logger& _logger;

        Camera _camera;
        const CameraConfig& _config;
    };
}

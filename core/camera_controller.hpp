#pragma once

#include <cstdint>

#include <SFML/System/Vector2.hpp>

#include "camera.hpp"
#include "event_manager.hpp"
#include "input_manager.hpp"
#include "logger.hpp"
#include "window.hpp"
#include "configs/camera_config.hpp"

namespace Aporia
{
    class CameraController
    {
    public:
        CameraController(Logger& logger, EventManager& event_manager, const CameraConfig& config);

        void control_movement(const InputManager& input_manager, float delta_time);
        void control_rotation(const InputManager& input_manager, float delta_time);
        void control_zoom(const InputManager& input_manager, float delta_time);
        void follow(const sf::Vector2f& to_follow, float delta_time);

        const Camera& get_camera() const;

    private:
        void _on_resize(Window& window, uint32_t width, uint32_t height);

        Logger& _logger;

        Camera _camera;
        const CameraConfig& _config;
    };
}

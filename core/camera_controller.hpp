#pragma once

#include <cstdint>

#include <glm/vec2.hpp>

#include "camera.hpp"
#include "event_manager.hpp"
#include "input_manager.hpp"
#include "logger.hpp"
#include "window.hpp"
#include "components/color.hpp"
#include "configs/camera_config.hpp"

namespace Aporia
{
    class CameraController
    {
    public:
        CameraController(Logger& logger, EventManager& event_manager, CameraConfig& config);

        void control_movement(const InputManager& input_manager, float delta_time);
        void control_rotation(const InputManager& input_manager, float delta_time);
        void control_zoom(const InputManager& input_manager, float delta_time);
        void follow(const glm::vec2& to_follow, float delta_time);

        Camera& get_camera();
        const Camera& get_camera() const;
        const Color& get_clear_color() const;

    private:
        Logger& _logger;

        Camera _camera;
        CameraConfig& _config;

        void _on_resize(Window& window, uint32_t width, uint32_t height);
        void _on_config_reload();
    };
}

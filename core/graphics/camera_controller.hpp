#pragma once

#include "aporia_types.hpp"
#include "graphics/camera.hpp"

namespace Aporia
{
    class Camera;
    class InputManager;
    class Window;
    struct CameraConfig;
    struct Color;

    class CameraController
    {
    public:
        CameraController(CameraConfig& config);

        void control_movement(const InputManager& input_manager, f32 delta_time);
        void control_rotation(const InputManager& input_manager, f32 delta_time);
        void control_zoom(const InputManager& input_manager, f32 delta_time);
        void follow(const v2& to_follow, f32 delta_time);

        Camera& get_camera();
        const Camera& get_camera() const;
        const Color& get_clear_color() const;

        void on_window_resize(u32 width, u32 height);
        void on_config_reload();

    private:
        Camera _camera;
        CameraConfig& _config;
    };
}

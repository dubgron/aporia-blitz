#pragma once

#include "aporia_types.hpp"
#include "graphics/camera.hpp"

namespace Aporia
{
    class Camera;
    class Window;
    struct CameraConfig;
    struct Color;

    class CameraController
    {
    public:
        CameraController(CameraConfig& config);

        void control_movement(f32 delta_time);
        void control_rotation(f32 delta_time);
        void control_zoom(f32 delta_time);
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

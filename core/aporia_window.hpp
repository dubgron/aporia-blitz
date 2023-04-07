#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;

namespace Aporia
{
    class Camera;
    class CameraController;
    struct WindowConfig;

    class Window final
    {
    public:
        Window(CameraController& camera);

        void init(const WindowConfig& config);
        void deinit();

        void display();

        void poll_events() const;

        void close();
        bool is_open() const;

        v2_u32 get_size() const;
        v2 get_mouse_position() const;

        GLFWwindow* get_native_window();

        void on_config_reload();

    private:
        const WindowConfig* _config;

        CameraController& _camera;

        GLFWwindow* _window;
    };
}

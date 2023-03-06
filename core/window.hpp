#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;

namespace Aporia
{
    class Camera;
    class CameraController;
    class InputManager;
    class Renderer;
    struct WindowConfig;

    class Window final
    {
    public:
        Window(InputManager& inputs, Renderer& renderer, CameraController& camera);

        void init(const WindowConfig& config);
        void deinit();

        void display();

        void poll_events() const;

        void close();
        bool is_open() const;

        v2_u32 get_size() const;
        v2 get_mouse_position() const;

        GLFWwindow* get_native_window();

        void on_config_reload(const WindowConfig& config);

    private:
        InputManager& _inputs;
        Renderer& _renderer;
        CameraController& _camera;

        GLFWwindow* _window;
    };
}

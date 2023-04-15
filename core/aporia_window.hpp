#pragma once

#include "aporia_types.hpp"

struct GLFWwindow;

namespace Aporia
{
    class Camera;

    class Window final
    {
    public:
        void init(Camera& camera);
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
        GLFWwindow* _window;
        Camera* _camera = nullptr;
    };
}

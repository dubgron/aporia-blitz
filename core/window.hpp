#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;

namespace Aporia
{
    class Camera;
    class EventManager;
    class Logger;
    struct WindowConfig;

    class Window final
    {
    public:
        Window(Logger& logger, EventManager& events, WindowConfig& config);
        ~Window();

        void display();

        void poll_events() const;

        void close();
        bool is_open() const;

        glm::uvec2 get_size() const;
        glm::vec2 get_mouse_position(Camera& camera) const;

        GLFWwindow* get_native_window();

    private:
        void _on_config_reload();

    private:
        Logger& _logger;
        EventManager& _events;

        WindowConfig& _config;

        GLFWwindow* _window;
    };
}

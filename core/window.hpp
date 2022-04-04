#pragma once

#include <memory>

#include "graphics/opengl.hpp"

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include "camera.hpp"
#include "event_manager.hpp"
#include "logger.hpp"
#include "components/color.hpp"
#include "configs/window_config.hpp"

namespace Aporia
{
    class Window final
    {
    public:
        Window(Logger& logger, EventManager& events, WindowConfig& config);
        ~Window();

        void clear(const Color& color = Colors::Black);
        void display();

        void poll_events() const;

        void close();
        bool is_open() const;

        glm::uvec2 get_size() const;
        glm::vec2 get_mouse_position(const Camera& camera) const;

        GLFWwindow* get_native_window();

    private:
        Logger& _logger;
        EventManager& _events;

        WindowConfig& _config;

        GLFWwindow* _window;

        void _on_config_reload();
    };
}

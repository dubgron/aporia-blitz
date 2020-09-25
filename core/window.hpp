#pragma once

#include <cstdint>
#include <memory>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include "event_manager.hpp"
#include "logger.hpp"
#include "components/color.hpp"
#include "configs/window_config.hpp"

namespace Aporia
{
    class Window final
    {
    public:
        Window(Logger& logger, EventManager& events, const WindowConfig& config);
        ~Window();

        void clear(const Color& color = Colors::Black);
        void display();

        void poll_events() const;

        void close();
        bool is_open() const;

        glm::uvec2 get_size() const;
        glm::vec2 get_mouse_position() const;

        GLFWwindow* get_native_window();

    private:
        Logger& _logger;
        EventManager& _events;

        GLFWwindow* _window;
    };
}

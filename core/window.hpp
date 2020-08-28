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

        friend void on_window_close(GLFWwindow* window);
        friend void on_resize(GLFWwindow* window, int32_t width, int32_t height);
        friend void on_set_key(GLFWwindow* window, int32_t key_code, int32_t scan_code, int32_t action, int32_t mods);
        friend void on_set_mouse_button(GLFWwindow* window, int32_t button_code, int32_t action, int32_t mods);
        friend void on_set_scroll(GLFWwindow* window, double x_offset, double y_offset);
        friend void on_set_cursor_position(GLFWwindow* window, double x_pos, double y_pos);
    };
}

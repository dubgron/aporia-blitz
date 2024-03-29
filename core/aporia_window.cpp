#include "aporia_window.hpp"

#include <glm/gtx/transform.hpp>

#include "aporia_debug.hpp"
#include "aporia_camera.hpp"
#include "aporia_config.hpp"
#include "aporia_game.hpp"
#include "aporia_input.hpp"
#include "aporia_rendering.hpp"
#include "aporia_utils.hpp"
#include "platform/aporia_opengl.hpp"

namespace Aporia
{
    Window* active_window = nullptr;

    void Window::display()
    {
        glfwSwapBuffers(handle);
    }

    void Window::poll_events() const
    {
        glfwPollEvents();
    }

    void Window::close()
    {
        glfwSetWindowShouldClose(handle, GLFW_TRUE);
    }

    bool Window::is_open() const
    {
        return !glfwWindowShouldClose(handle);
    }

    v2 Window::get_mouse_screen_position() const
    {
        v2_f64 screen_position{ 0.0 };
        glfwGetCursorPos(handle, &screen_position.x, &screen_position.y);

        return v2{ screen_position.x, height - screen_position.y };
    }

    v2 Window::get_mouse_world_position() const
    {
        v2 screen_position = get_mouse_screen_position();

        // @NOTE(dubgron): Precalculated following lines:
        //     screen_to_clip = glm::scale(glm::mat4{ 1.f }, glm::vec3{ 2.f / width, 2.f / height, -1.f });
        //     screen_to_clip = glm::translate(screen_to_clip, glm::vec3{ -1.f, -1.f, 0.f });
        m4 screen_to_clip{
            2.f / width,    0.f,            0.f,    0.f,
            0.f,            2.f / height,   0.f,    0.f,
            0.f,            0.f,            -1.f,   0.f,
            -1.f,           -1.f,           0.f,    1.f };

        m4 view_projection_matrix = active_camera->calculate_view_projection_matrix();
        m4 clip_to_world = glm::inverse(view_projection_matrix);
        v2 world_position = clip_to_world * screen_to_clip * v4{ screen_position, 0.f, 1.f };

        return world_position;
    }

    void Window::apply_config()
    {
        APORIA_ASSERT(window_config.width > 0 && window_config.height > 0);

        // @NOTE(dubgron): This will trigger the FramebufferSizeCallback function,
        // if the size differs from the current one.
        glfwSetWindowSize(handle, window_config.width, window_config.height);

        glfwSetWindowTitle(handle, *window_config.title);
        glfwSwapInterval(window_config.vsync);

        if (window_config.position != WindowConfig::INVALID_POSITION)
        {
            glfwSetWindowPos(handle, window_config.position.x, window_config.position.y);
        }
    }

    Window* create_window(MemoryArena* arena)
    {
        glfwSetErrorCallback([](i32 error, CString description)
        {
            APORIA_LOG(Error, "GLFW Error #%: %", error, description);
        });

        if (!glfwInit())
        {
            APORIA_LOG(Critical, "Failed to initialize GLFW!");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);

        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        GLFWwindow* handle = glfwCreateWindow(window_config.width, window_config.height, *window_config.title, nullptr, nullptr);

        if (!handle)
        {
            glfwTerminate();
            APORIA_LOG(Critical, "Failed to initialize GLFW window!");
            return nullptr;
        }

        Window* result = arena_push_uninitialized<Window>(arena);
        result->handle = handle;
        result->width = window_config.width;
        result->height = window_config.height;

        glfwMakeContextCurrent(handle);
        glfwSetWindowUserPointer(handle, result);
        glfwSwapInterval(window_config.vsync);

        if (window_config.position != WindowConfig::INVALID_POSITION)
        {
            glfwSetWindowPos(handle, window_config.position.x, window_config.position.y);
        }

        glfwSetWindowCloseCallback(handle, [](GLFWwindow* handle)
            {
                Window* window = (Window*)glfwGetWindowUserPointer(handle);
                window->close();
            });

        glfwSetKeyCallback(handle, [](GLFWwindow* handle, i32 key_code, i32 scan_code, i32 action, i32 mods)
            {
                Key key = (Key)key_code;
                InputAction input_action = (InputAction)action;
                input_process_key_event(key, input_action);
            });

        glfwSetMouseButtonCallback(handle, [](GLFWwindow* handle, i32 button_code, i32 action, i32 mods)
            {
                MouseButton button = (MouseButton)button_code;
                InputAction input_action = (InputAction)action;
                input_process_mouse_event(button, input_action);
            });

        glfwSetScrollCallback(handle, [](GLFWwindow* handle, f64 x_offset, f64 y_offset)
            {
                input_process_scroll_event(Mouse_HorizontalWheel, x_offset);
                input_process_scroll_event(Mouse_VerticalWheel, y_offset);
            });

        // @NOTE(dubgron): We probably would like to use this callback in the future.
        // glfwSetCursorPosCallback(handle, [](GLFWwindow* handle, f64 x_pos, f64 y_pos)
        // {
        // });

        glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* handle, i32 width, i32 height)
        {
            if (width > 0 && height > 0)
            {
                active_window->width = width;
                active_window->height = height;

                active_camera->adjust_aspect_ratio_to_render_surface();

                adjust_framebuffers_to_render_surface();
            }
        });

        return result;
    }

    void destroy_active_window()
    {
        if (active_window)
        {
            glfwMakeContextCurrent(active_window->handle);
            glfwDestroyWindow(active_window->handle);
        }
        glfwTerminate();
    }
}

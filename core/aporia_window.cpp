#include "aporia_window.hpp"

#include <glm/gtx/transform.hpp>

#include "aporia_debug.hpp"
#include "aporia_camera.hpp"
#include "aporia_config.hpp"
#include "aporia_game.hpp"
#include "aporia_inputs.hpp"
#include "aporia_rendering.hpp"
#include "aporia_utils.hpp"
#include "platform/aporia_opengl.hpp"

namespace Aporia
{
    extern Input input;

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

    v2_u32 Window::get_size() const
    {
        v2_i32 size;
        glfwGetWindowSize(handle, &size.x, &size.y);

        return size;
    }

    v2 Window::get_mouse_position() const
    {
        v2_f64 screen_position{ 0.0 };
        glfwGetCursorPos(handle, &screen_position.x, &screen_position.y);

        const v2 window_size = get_size();

        // @NOTE(dubgron): Precalculated following lines:
        //     screen_to_clip = glm::scale(glm::mat4{ 1.f }, glm::vec3{ 2.f / window_size.x, -2.f / window_size.y, 1.f });
        //     screen_to_clip = glm::translate(screen_to_clip, glm::vec3{ -1.f, 1.f, 0.f });
        const m4 screen_to_clip{
            2.f / window_size.x,   0.f,                   0.f,   0.f,
            0.f,                   -2.f / window_size.y,  0.f,   0.f,
            0.f,                   0.f,                   1.f,   0.f,
            -1.f,                  1.f,                   0.f,   1.f };

        const m4 view_projection_matrix = active_camera->calculate_view_projection_matrix();
        const m4 clip_to_world = glm::inverse(view_projection_matrix);
        const v2 world_position = clip_to_world * screen_to_clip * v4{ screen_position, 0.f, 1.f };

        return world_position;
    }

    void Window::apply_config()
    {
        APORIA_ASSERT(window_config.width > 0 && window_config.height > 0);

        active_window->width = width;
        active_window->height = height;

        glfwSetWindowTitle(handle, *window_config.title);
        glfwSetWindowSize(handle, window_config.width, window_config.height);
        glfwSwapInterval(window_config.vsync);

        if (window_config.position != WindowConfig::INVALID_POSITION)
        {
            glfwSetWindowPos(handle, window_config.position.x, window_config.position.y);
        }
    }

    Window* create_window(MemoryArena* arena)
    {
        glfwSetErrorCallback([](i32 error, const char* description)
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

        Window* result = arena->push<Window>();
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
            Window& window = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(handle));
            window.close();
        });

        glfwSetKeyCallback(handle, [](GLFWwindow* handle, i32 key_code, i32 scan_code, i32 action, i32 mods)
        {
            const Key key = static_cast<Key>(key_code);
            const InputAction input_action = static_cast<InputAction>(action);

            if (key != Key::Unknown)
            {
                process_input_action(input.keys[key_code], input_action);
            }
            else switch (input_action)
            {
                case InputAction::Released: APORIA_LOG(Warning, "The unknown key has been released!");  break;
                case InputAction::Pressed:  APORIA_LOG(Warning, "The unknown key has been pressed!");   break;
                case InputAction::Repeat:   APORIA_LOG(Warning, "The unknown key has been held!");      break;
            }
        });

        glfwSetMouseButtonCallback(handle, [](GLFWwindow* handle, i32 button_code, i32 action, i32 mods)
        {
            const InputAction input_action = static_cast<InputAction>(action);
            process_input_action(input.mouse[button_code], input_action);
        });

        glfwSetScrollCallback(handle, [](GLFWwindow* handle, f64 x_offset, f64 y_offset)
        {
            process_input_value(input.wheels[+MouseWheel::HorizontalWheel], x_offset);
            process_input_value(input.wheels[+MouseWheel::VerticalWheel], y_offset);
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

                active_camera->refresh_aspect_ratio();

                refresh_framebuffers();
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

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

Window* active_window = nullptr;

void window_create(MemoryArena* arena)
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
        return;
    }

    active_window = arena_push_uninitialized<Window>(arena);
    active_window->handle = handle;
    active_window->width = window_config.width;
    active_window->height = window_config.height;

    glfwMakeContextCurrent(handle);
    glfwSwapInterval(window_config.vsync);

    if (window_config.position != WindowConfig::INVALID_POSITION)
    {
        glfwSetWindowPos(handle, window_config.position.x, window_config.position.y);
    }

    glfwSetWindowCloseCallback(handle, [](GLFWwindow* handle)
        {
            window_close();
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

    glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* handle, i32 width, i32 height)
        {
            if (width > 0 && height > 0)
            {
                active_window->width = width;
                active_window->height = height;
            }
        });
}

void window_destroy()
{
    if (active_window)
    {
        glfwMakeContextCurrent(active_window->handle);
        glfwDestroyWindow(active_window->handle);
    }
    glfwTerminate();
}

void window_display()
{
    glfwSwapBuffers(active_window->handle);
}

void window_poll_events()
{
    glfwPollEvents();
}

void window_close()
{
    glfwSetWindowShouldClose(active_window->handle, GLFW_TRUE);
}

bool window_is_open()
{
    return !glfwWindowShouldClose(active_window->handle);
}

void window_apply_config()
{
    APORIA_ASSERT(window_config.width > 0 && window_config.height > 0);

    // @NOTE(dubgron): This will trigger the FramebufferSizeCallback function,
    // if the size differs from the current one.
    glfwSetWindowSize(active_window->handle, window_config.width, window_config.height);

    glfwSetWindowTitle(active_window->handle, *window_config.title);
    glfwSwapInterval(window_config.vsync);

    if (window_config.position != WindowConfig::INVALID_POSITION)
    {
        glfwSetWindowPos(active_window->handle, window_config.position.x, window_config.position.y);
    }
}

v2 get_mouse_screen_position()
{
    v2_f64 screen_position{ 0.0 };
    glfwGetCursorPos(active_window->handle, &screen_position.x, &screen_position.y);

    return v2{ screen_position.x, active_window->height - screen_position.y };
}

v2 get_mouse_viewport_position()
{
    v2 mouse_viewport_position = get_mouse_screen_position();
    mouse_viewport_position -= viewport_offset;
    return mouse_viewport_position;
}

v2 get_mouse_world_position()
{
    v2 mouse_viewport_position = get_mouse_viewport_position();

    // @NOTE(dubgron): Precalculated following lines:
    //     viewport_to_clip = glm::scale(glm::mat4{ 1.f }, glm::vec3{ 2.f / width, 2.f / height, -1.f });
    //     viewport_to_clip = glm::translate(viewport_to_clip, glm::vec3{ -1.f, -1.f, 0.f });
    m4 viewport_to_clip{
        2.f / viewport_width, 0.f, 0.f, 0.f,
        0.f, 2.f / viewport_height, 0.f, 0.f,
        0.f, 0.f, -1.f, 0.f,
        -1.f, -1.f, 0.f, 1.f };

    m4 view_projection_matrix = active_camera.calculate_view_projection_matrix();
    m4 clip_to_world = glm::inverse(view_projection_matrix);
    v2 world_position = clip_to_world * viewport_to_clip * v4{ mouse_viewport_position, 0.f, 1.f };

    return world_position;
}

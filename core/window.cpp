#include "window.hpp"

#include <cstdint>
#include <functional>

#include <glm/gtx/transform.hpp>

#include "aporia_config.hpp"
#include "aporia_inputs.hpp"
#include "aporia_rendering.hpp"
#include "common.hpp"
#include "graphics/camera.hpp"
#include "graphics/camera_controller.hpp"
#include "platform/opengl.hpp"

namespace Aporia
{
    Window::Window(CameraController& camera)
        : _camera(camera)
    {
    }

    void Window::init(const WindowConfig& config)
    {
        _config = &config;

        glfwSetErrorCallback([](i32 error, const char* description)
            {
                APORIA_LOG(Error, "GLFW Error #{}: {}", error, description);
            });

        if (!glfwInit())
        {
            APORIA_LOG(Critical, "Failed to initialize GLFW!");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);

        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        _window = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);

        if (!_window)
        {
            glfwTerminate();
        }

        glfwMakeContextCurrent(_window);
        glfwSetWindowUserPointer(_window, this);
        glfwSwapInterval(config.vsync);

        if (config.position)
        {
            glfwSetWindowPos(_window, config.position->x, config.position->y);
        }

        glfwSetWindowCloseCallback(_window, [](GLFWwindow* handle)
            {
                Window& window = *reinterpret_cast<Window*>( glfwGetWindowUserPointer(handle) );
                window.close();
            });

        glfwSetKeyCallback(_window, [](GLFWwindow* handle, i32 key_code, i32 scan_code, i32 action, i32 mods)
            {
                const Keyboard key = static_cast<Keyboard>(key_code);
                if (action == GLFW_PRESS || action == GLFW_REPEAT)
                {
                    on_key_triggered(key);
                }
                else if (action == GLFW_RELEASE)
                {
                    on_key_released(key);
                }
            });

        glfwSetMouseButtonCallback(_window, [](GLFWwindow* handle, i32 button_code, i32 action, i32 mods)
            {
                const Mouse button = static_cast<Mouse>(button_code);
                if (action == GLFW_PRESS || action == GLFW_REPEAT)
                {
                    on_button_triggered(button);
                }
                else if (action == GLFW_RELEASE)
                {
                    on_button_released(button);
                }
            });

        glfwSetScrollCallback(_window, [](GLFWwindow* handle, f64 x_offset, f64 y_offset)
            {
                on_wheel_scrolled(MouseWheel::HorizontalWheel, static_cast<f32>(x_offset));
                on_wheel_scrolled(MouseWheel::VerticalWheel, static_cast<f32>(y_offset));
            });

        // @NOTE(dubgron): We probably would like to use this callback in the future.
        // glfwSetCursorPosCallback(_window, [](GLFWwindow* handle, f64 x_pos, f64 y_pos)
        //     {
        //     });

        glfwSetFramebufferSizeCallback(_window, [](GLFWwindow* handle, i32 width, i32 height)
            {
                const Window& window = *reinterpret_cast<Window*>( glfwGetWindowUserPointer(handle) );
                resize_framebuffers(width, height);
                window._camera.on_window_resize(width, height);
                glViewport(0, 0, width, height);
            });

#if !defined(APORIA_EMSCRIPTEN)
        // @NOTE(dubgron): It has to be called after glfwMakeContextCurrent.
        const i32 gl3w_init_return_code = gl3wInit();
        if (gl3w_init_return_code != GL3W_OK)
        {
            const char* error_reason = gl3w_return_code_to_string(gl3w_init_return_code);
            APORIA_LOG(Critical, "Failed to initialize OpenGL! Reason: {}", error_reason);
            return;
        }

        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
            {
                const LogLevel log_level = gl_debug_severity_to_log_level(severity);
                const char* debug_source = gl_debug_source_to_string(source);
                const char* debug_type = gl_debug_type_to_string(type);
                APORIA_LOG(log_level, "{} {} [ID: {}] '{}'", debug_source, debug_type, id, message);
            }, nullptr);
#endif

        APORIA_LOG(Info, reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    }

    void Window::deinit()
    {
        glfwMakeContextCurrent(_window);
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    void Window::display()
    {
        glfwSwapBuffers(_window);
    }

    void Window::poll_events() const
    {
        glfwPollEvents();
    }

    void Window::close()
    {
        glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }

    bool Window::is_open() const
    {
        return !glfwWindowShouldClose(_window);
    }

    v2_u32 Window::get_size() const
    {
        v2_i32 size;
        glfwGetWindowSize(_window, &size.x, &size.y);

        return size;
    }

    v2 Window::get_mouse_position() const
    {
        v2_f64 screen_position{ 0.0 };
        glfwGetCursorPos(_window, &screen_position.x, &screen_position.y);

        const v2 window_size = get_size();

        /**
         *  Precalculated following lines:
         *
         *  screen_to_clip = glm::scale(glm::mat4{ 1.f }, glm::vec3{ 2.f / window_size.x, -2.f / window_size.y, 1.f });
         *  screen_to_clip = glm::translate(screen_to_clip, glm::vec3{ -1.f, 1.f, 0.f });
         *
         */
        const m4 screen_to_clip{
            2.f / window_size.x,   0.f,                   0.f,   0.f,
            0.f,                   -2.f / window_size.y,  0.f,   0.f,
            0.f,                   0.f,                   1.f,   0.f,
            -1.f,                  1.f,                   0.f,   1.f };

        const m4 clip_to_world = glm::inverse(_camera.get_camera().get_view_projection_matrix());
        const v2 world_position = clip_to_world * screen_to_clip * v4{ screen_position, 0.f, 1.f };

        return world_position;
    }

    GLFWwindow* Window::get_native_window()
    {
        return _window;
    }

    void Window::on_config_reload()
    {
        glfwSetWindowTitle(_window, _config->title.c_str());
        glfwSetWindowSize(_window, _config->width, _config->height);
        glfwSwapInterval(_config->vsync);

        if (_config->position)
        {
            glfwSetWindowPos(_window, _config->position->x, _config->position->y);
        }
    }
}

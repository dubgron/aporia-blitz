#include "window.hpp"

#include <cstdint>
#include <functional>

#include <glm/gtx/transform.hpp>

#include "inputs/all_inputs.hpp"

namespace Aporia
{
    Window::Window(Logger& logger, EventManager& events, WindowConfig& config)
        : _logger(logger), _events(events), _config(config)
    {
        glfwSetErrorCallback([](int32_t error, const char* description) { fprintf(stderr, "[GLFW Error #%d]: %s\n", error, description); });

        if (!glfwInit())
        {
            _logger.log(LOG_CRITICAL) << "Failed to initialize GLFW!";
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

        if (_config.position)
        {
            glfwSetWindowPos(_window, _config.position->x, _config.position->y);
        }

        glfwSetWindowCloseCallback(_window, [](GLFWwindow * window)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);
                win._events.call_event<WindowCloseEvent>(win);
            });

        glfwSetKeyCallback(_window, [](GLFWwindow* window, int32_t key_code, int32_t scan_code, int32_t action, int32_t mods)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);
                Keyboard key = static_cast<Keyboard>(key_code);
                if (action == GLFW_PRESS || action == GLFW_REPEAT)
                    win._events.call_event<KeyPressedEvent>(key);
                else if (action == GLFW_RELEASE)
                    win._events.call_event<KeyReleasedEvent>(key);
            });

        glfwSetMouseButtonCallback(_window, [](GLFWwindow* window, int32_t button_code, int32_t action, int32_t mods)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);
                Mouse button = static_cast<Mouse>(button_code);
                if (action == GLFW_PRESS || action == GLFW_REPEAT)
                    win._events.call_event<ButtonPressedEvent>(button);
                else if (action == GLFW_RELEASE)
                    win._events.call_event<ButtonReleasedEvent>(button);
            });

        glfwSetScrollCallback(_window, [](GLFWwindow* window, double x_offset, double y_offset)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);
                if (x_offset)
                    win._events.call_event<MouseWheelScrollEvent>(MouseWheel::HorizontalWheel, static_cast<float>(x_offset));
                if (y_offset)
                    win._events.call_event<MouseWheelScrollEvent>(MouseWheel::VerticalWheel, static_cast<float>(y_offset));
            });

        glfwSetCursorPosCallback(_window, [](GLFWwindow* window, double x_pos, double y_pos)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);
                win._events.call_event<MouseMoveEvent>(glm::vec2{ x_pos, y_pos });
            });

        glfwSetFramebufferSizeCallback(_window, [](GLFWwindow* window, int32_t width, int32_t height)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);
                win._events.call_event<WindowResizeEvent>(win, width, height);
            });

#       if !defined(APORIA_EMSCRIPTEN)
            if (gl3wInit())
            {
                _logger.log(LOG_CRITICAL) << "Failed to initialize OpenGL!";
            }
#       endif

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        _logger.log(LOG_INFO) << glGetString(GL_VERSION);

#       if !defined(APORIA_EMSCRIPTEN)
            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
                {
                    const Aporia::Logger* logger = reinterpret_cast<const Aporia::Logger*>(userParam);
                    const auto get_logger = [&logger](GLenum severity)
                    {
                        switch (severity)
                        {
                        case GL_DEBUG_SEVERITY_HIGH:            return logger->log(LOG_ERROR);
                        case GL_DEBUG_SEVERITY_MEDIUM:          return logger->log(LOG_WARNING);
                        case GL_DEBUG_SEVERITY_LOW:             return logger->log(LOG_WARNING);
                        case GL_DEBUG_SEVERITY_NOTIFICATION:    return logger->log(LOG_DEBUG);
                        default:                                return logger->log(LOG_ERROR);
                        }
                    };

                    get_logger(severity) << get_debug_source(source) << " " << get_debug_type(type) << " [ID=" << id << "] \"" << message << "\"";
                },
                reinterpret_cast<const void*>(&_logger));
#       endif

        events.add_listener<WindowCloseEvent>([](Window& window)
            {
                window.close();
            });

        events.add_listener<WindowResizeEvent>([](Window& window, uint32_t width, uint32_t height)
            {
                glViewport(0, 0, width, height);
            });

        events.add_listener<ReloadWindowConfigEvent>(std::bind(&Window::_on_config_reload, this));
    }

    Window::~Window()
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
        _events.call_event<BeginProcessingWindowEvents>();

        glfwPollEvents();

        _events.call_event<EndProcessingWindowEvents>();
    }

    void Window::close()
    {
        glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }

    bool Window::is_open() const
    {
        return !glfwWindowShouldClose(_window);
    }

    glm::uvec2 Window::get_size() const
    {
        glm::ivec2 size;
        glfwGetWindowSize(_window, &size.x, &size.y);

        return size;
    }

    glm::vec2 Window::get_mouse_position(Camera& camera) const
    {
        glm::dvec2 screen_position{ 0.0 };
        glfwGetCursorPos(_window, &screen_position.x, &screen_position.y);

        const glm::vec2 window_size = get_size();

        /**
         *  Precalculated following lines:
         *
         *  screen_to_clip = glm::scale(glm::mat4{ 1.0f }, glm::vec3{ 2.0f / window_size.x, -2.0f / window_size.y, 1.0f });
         *  screen_to_clip = glm::translate(screen_to_clip, glm::vec3{ -1.0f, 1.0f, 0.0f });
         *
         */
        const glm::mat4 screen_to_clip{
            2.0f / window_size.x,   0.0f,                   0.0f,   0.0f,
            0.0f,                   -2.0f / window_size.y,  0.0f,   0.0f,
            0.0f,                   0.0f,                   1.0f,   0.0f,
            -1.0f,                  1.0f,                   0.0f,   1.0f };

        const glm::mat4 clip_to_world = glm::inverse(camera.get_view_projection_matrix());
        const glm::vec2 world_position = clip_to_world * screen_to_clip * glm::vec4{ screen_position, 0.0f, 1.0f };

        return world_position;
    }

    GLFWwindow* Window::get_native_window()
    {
        return _window;
    }

    void Window::_on_config_reload()
    {
        glfwSetWindowTitle(_window, _config.title.c_str());
        glfwSetWindowSize(_window, _config.width, _config.height);
        glfwSwapInterval(_config.vsync);

        if (_config.position)
        {
            glfwSetWindowPos(_window, _config.position->x, _config.position->y);
        }
    }
}

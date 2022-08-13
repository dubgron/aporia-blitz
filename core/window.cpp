#include "window.hpp"

#include <cstdint>
#include <functional>

#include <glm/gtx/transform.hpp>

#include "common.hpp"
#include "event_manager.hpp"
#include "configs/window_config.hpp"
#include "graphics/camera.hpp"
#include "inputs/all_inputs.hpp"
#include "platform/opengl.hpp"

namespace Aporia
{
    Window::Window(Logger& logger, EventManager& events, WindowConfig& config)
        : _logger(logger), _events(events), _config(config)
    {
        glfwSetErrorCallback([](int32_t error, const char* description) { fprintf(stderr, "[GLFW Error #%d]: %s\n", error, description); });

        if (!glfwInit())
        {
            APORIA_LOG(_logger, Critical, "Failed to initialize GLFW!");
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
                {
                    win._events.call_event<KeyPressedEvent>(key);
                }
                else if (action == GLFW_RELEASE)
                {
                    win._events.call_event<KeyReleasedEvent>(key);
                }
            });

        glfwSetMouseButtonCallback(_window, [](GLFWwindow* window, int32_t button_code, int32_t action, int32_t mods)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);
                Mouse button = static_cast<Mouse>(button_code);
                if (action == GLFW_PRESS || action == GLFW_REPEAT)
                {
                    win._events.call_event<ButtonPressedEvent>(button);
                }
                else if (action == GLFW_RELEASE)
                {
                    win._events.call_event<ButtonReleasedEvent>(button);
                }
            });

        glfwSetScrollCallback(_window, [](GLFWwindow* window, double x_offset, double y_offset)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);
                if (x_offset)
                {
                    win._events.call_event<MouseWheelScrollEvent>(MouseWheel::HorizontalWheel, static_cast<float>(x_offset));
                }
                if (y_offset)
                {
                    win._events.call_event<MouseWheelScrollEvent>(MouseWheel::VerticalWheel, static_cast<float>(y_offset));
                }
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
                APORIA_LOG(_logger, Critical, "Failed to initialize OpenGL!");
            }
#       endif

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        APORIA_LOG(_logger, Info, reinterpret_cast<const char*>(glGetString(GL_VERSION)));

#       if !defined(APORIA_EMSCRIPTEN)
            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
                {
                    constexpr auto log_level = [](GLenum severity)
                    {
                        switch (severity)
                        {
                        case GL_DEBUG_SEVERITY_HIGH:            return LogLevel::Error;
                        case GL_DEBUG_SEVERITY_MEDIUM:          return LogLevel::Warning;
                        case GL_DEBUG_SEVERITY_LOW:             return LogLevel::Info;
                        case GL_DEBUG_SEVERITY_NOTIFICATION:    return LogLevel::Debug;
                        default:                                return LogLevel::Critical;
                        }
                    };

                    constexpr auto debug_source = [](GLenum source)
                    {
                        switch (source)
                        {
                        case GL_DEBUG_SOURCE_API:                   return "OPENGL_API";
                        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:         return "WINDOW_SYSTEM";
                        case GL_DEBUG_SOURCE_SHADER_COMPILER:       return "SHADER_COMPILER";
                        case GL_DEBUG_SOURCE_THIRD_PARTY:           return "THIRD_PARTY";
                        case GL_DEBUG_SOURCE_APPLICATION:           return "APPLICATION";
                        case GL_DEBUG_SOURCE_OTHER:                 return "OTHER";
                        default:                                    return "INVALID_SOURCE";
                        }
                    };

                    constexpr auto debug_type = [](GLenum type)
                    {
                        switch (type)
                        {
                        case GL_DEBUG_TYPE_ERROR:                   return "ERROR";
                        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:     return "DEPRECATED_BEHAVIOR";
                        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:      return "UNDEFINED_BEHAVIOR";
                        case GL_DEBUG_TYPE_PORTABILITY:             return "PORTABILITY";
                        case GL_DEBUG_TYPE_PERFORMANCE:             return "PERFORMANCE";
                        case GL_DEBUG_TYPE_MARKER:                  return "MARKER";
                        case GL_DEBUG_TYPE_PUSH_GROUP:              return "PUSH_GROUP";
                        case GL_DEBUG_TYPE_POP_GROUP:               return "POP_GROUP";
                        case GL_DEBUG_TYPE_OTHER:                   return "OTHER";
                        default:                                    return "INVALID_TYPE";
                        }
                    };

                    static Logger logger{ "OpenGL" };
                    APORIA_LOG(logger, log_level(severity), "{} {} [ID = {}] '{}'", debug_source(source), debug_type(type), id, message);
                }, nullptr);
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

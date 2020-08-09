#include "window.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/VideoMode.hpp>

#include "inputs/all_inputs.hpp"

namespace Aporia
{
    Window::Window(Logger& logger, EventManager& events, const WindowConfig& config)
        : _logger(logger), _events(events)
    {
        glfwSetErrorCallback([](int32_t error, const char* description) { fprintf(stderr, "[GLFW Error #%d]: %s\n", error, description); });

        if (!glfwInit())
            _logger.log(LOG_CRITICAL) << "Failed to initialize GLFW!";

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        _window = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);

        if (!_window)
            glfwTerminate();

        glfwMakeContextCurrent(_window);
        glfwSetWindowUserPointer(_window, this);
        glfwSwapInterval(config.vsync);

        glfwSetWindowCloseCallback(_window, on_window_close);
        glfwSetKeyCallback(_window, on_set_key);
        glfwSetMouseButtonCallback(_window, on_set_mouse_button);
        glfwSetScrollCallback(_window, on_set_scroll);
        glfwSetCursorPosCallback(_window, on_set_cursor_position);
        glfwSetFramebufferSizeCallback(_window, on_resize);

        if (gl3wInit())
            _logger.log(LOG_CRITICAL) << "Failed to initialize OpenGL!";

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        _logger.log(LOG_INFO) << glGetString(GL_VERSION);

        events.add_listener<WindowCloseEvent>([](Window& window)
            {
                window.close();
            });

        events.add_listener<WindowResizeEvent>([](Window& window, uint32_t width, uint32_t height)
            {
                glViewport(0, 0, width, height);
            });
    }

    Window::~Window()
    {
        glfwMakeContextCurrent(_window);
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    void Window::clear(const Color& color)
    {
        glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

    glm::vec2 Window::get_mouse_position() const
    {
        glm::dvec2 position;
        glfwGetCursorPos(_window, &position.x, &position.y);

        return position;
    }

    GLFWwindow* Window::get_native_window()
    {
        return _window;
    }

    void on_window_close(GLFWwindow* window)
    {
        Window& win = *(Window*)glfwGetWindowUserPointer(window);

        win._events.call_event<WindowCloseEvent>(win);
    }

    void on_resize(GLFWwindow* window, int32_t width, int32_t height)
    {
        Window& win = *(Window*)glfwGetWindowUserPointer(window);

        win._events.call_event<WindowResizeEvent>(win, width, height);
    }

    void on_set_key(GLFWwindow* window, int32_t key_code, int32_t scan_code, int32_t action, int32_t mods)
    {
        Window& win = *(Window*)glfwGetWindowUserPointer(window);

        Keyboard key = static_cast<Keyboard>(key_code);
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            win._events.call_event<KeyPressedEvent>(key);
        else if (action == GLFW_RELEASE)
            win._events.call_event<KeyReleasedEvent>(key);
    }

    void on_set_mouse_button(GLFWwindow* window, int32_t button_code, int32_t action, int32_t mods)
    {
        Window& win = *(Window*)glfwGetWindowUserPointer(window);

        Mouse button = static_cast<Mouse>(button_code);
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            win._events.call_event<ButtonPressedEvent>(button);
        else if (action == GLFW_RELEASE)
            win._events.call_event<ButtonReleasedEvent>(button);
    }

    void on_set_scroll(GLFWwindow* window, double x_offset, double y_offset)
    {
        Window& win = *(Window*)glfwGetWindowUserPointer(window);

        if (x_offset)
            win._events.call_event<MouseWheelScrollEvent>(MouseWheel::HorizontalWheel, x_offset);

        if (y_offset)
            win._events.call_event<MouseWheelScrollEvent>(MouseWheel::VerticalWheel, y_offset);
    }

    void on_set_cursor_position(GLFWwindow* window, double x_pos, double y_pos)
    {
        Window& win = *(Window*)glfwGetWindowUserPointer(window);

        win._events.call_event<MouseMoveEvent>(glm::vec2{ x_pos, y_pos });
    }
}

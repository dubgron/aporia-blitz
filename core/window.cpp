#include "window.hpp"

namespace Aporia
{
    Window::Window(Logger& logger, const WindowConfig& config)
        : _logger(logger), _config(config)
    {
        open();
    }

    Window::~Window()
    {
        _window.close();
    }

    void Window::open()
    {
        if (is_open())
        {
            _logger.log(LOG_WARNING) << "Window '" << _config.title << "' is already open!";
        }
        else
        {
            _window.create(sf::VideoMode(_config.width, _config.height), _config.title);
            _window.setPosition(_config.position);
            _window.setFramerateLimit(_config.framerate);
            _visible = true;

            _window.setView(sf::View(sf::FloatRect(-1.0f, 1.0f, 2.0f, -2.0f)));
        }
    }

    void Window::close()
    {
        if (is_open())
        {
            _window.close();
            _visible = false;
        }
        else
        {
            _logger.log(LOG_WARNING) << "Window '" << _config.title << "' is already closed!";
        }
    }

    void Window::show()
    {
        if (is_visible())
        {
            _logger.log(LOG_WARNING) << "Window '" << _config.title << "' is already visible!";
        }
        else
        {
            _visible = true;
            _window.setVisible(true);
        }
    }

    void Window::hide()
    {
        if (is_visible())
        {
            _visible = false;
            _window.setVisible(false);
        }
        else
        {
            _logger.log(LOG_WARNING) << "Window '" << _config.title << "' is already hidden!";
        }
    }
    
    void Window::clear(const sf::Color& color)
    {
        _window.clear(color);
    }

    void Window::draw(const VertexArray& vertex_array, sf::RenderStates states)
    {
        _window.draw(vertex_array, states);
    }

    void Window::display()
    {
        _window.display();
    }

    bool Window::poll_event(sf::Event& event)
    {
        return _window.pollEvent(event);
    }

    bool Window::is_open() const
    {
        return _window.isOpen();
    }

    bool Window::is_visible() const
    {
        return _visible;
    }
}

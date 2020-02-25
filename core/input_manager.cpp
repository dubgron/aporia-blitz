#include "input_manager.hpp"

#include <functional>

namespace Aporia
{
    InputManager::InputManager(const std::shared_ptr<Logger>& logger, const std::shared_ptr<EventManager>& event_manager)
        : _logger(logger), _keys(logger), _buttons(logger), _wheels(logger)
    {
        using namespace std::placeholders;

        event_manager->add_listener<BeginProcessingWindowEvents>(std::bind(&InputManager::_reset_wheel, this));

        event_manager->add_listener<KeyPressedEvent>(std::bind(&InputManager::_on_key_triggered, this, _1));
        event_manager->add_listener<KeyReleasedEvent>(std::bind(&InputManager::_on_key_released, this, _1));

        event_manager->add_listener<ButtonPressedEvent>(std::bind(&InputManager::_on_button_triggered, this, _1));
        event_manager->add_listener<ButtonReleasedEvent>(std::bind(&InputManager::_on_button_released, this, _1));

        event_manager->add_listener<MouseWheelScrollEvent>(std::bind(&InputManager::_on_wheel_scrolled, this, _1, _2));

        event_manager->add_listener<EndProcessingWindowEvents>(std::bind(&InputBuffer<Keyboard>::update, &_keys));
        event_manager->add_listener<EndProcessingWindowEvents>(std::bind(&InputBuffer<Mouse>::update, &_buttons));
        event_manager->add_listener<EndProcessingWindowEvents>(std::bind(&InputBuffer<MouseWheel>::update, &_wheels));
    }

    bool InputManager::is_key_triggered(Keyboard key) const
    {
        return _keys.is_triggered(key);
    }

    bool InputManager::is_key_pressed(Keyboard key) const
    {
        return _keys.is_pressed(key);
    }

    bool InputManager::is_key_released(Keyboard key) const
    {
        return _keys.is_released(key);
    }

    bool InputManager::is_any_key_triggered() const
    {
        return _keys.is_any_triggered();
    }

    bool InputManager::is_any_key_pressed() const
    {
        return _keys.is_any_pressed();
    }

    bool InputManager::is_any_key_released() const
    {
        return _keys.is_any_released();
    }

    bool InputManager::is_button_triggered(Mouse button) const
    {
        return _buttons.is_triggered(button);
    }

    bool InputManager::is_button_pressed(Mouse button) const
    {
        return _buttons.is_pressed(button);
    }

    bool InputManager::is_button_released(Mouse button) const
    {
        return _buttons.is_released(button);
    }

    bool InputManager::is_any_button_triggered() const
    {
        return _buttons.is_any_triggered();
    }

    bool InputManager::is_any_button_pressed() const
    {
        return _buttons.is_any_pressed();
    }

    bool InputManager::is_any_button_released() const
    {
        return _buttons.is_any_released();
    }

    sf::Vector2i InputManager::get_mouse_position() const
    {
        return sf::Mouse::getPosition();
    }

    float InputManager::is_wheel_scrolling(MouseWheel wheel) const
    {
        return _wheels.is_triggered(wheel) * _wheel_delta;
    }

    void InputManager::_on_key_triggered(Keyboard key)
    {
        _keys.push_state(key, true);
    }

    void InputManager::_on_key_released(Keyboard key)
    {
        _keys.push_state(key, false);
    }

    void InputManager::_on_button_triggered(Mouse button)
    {
        _buttons.push_state(button, true);
    }

    void InputManager::_on_button_released(Mouse button)
    {
        _buttons.push_state(button, false);
    }

    void InputManager::_on_wheel_scrolled(MouseWheel wheel, float delta)
    {
        _wheel_delta = delta;
        _wheels.push_state(wheel, true);
    }

    void InputManager::_reset_wheel()
    {
        _wheel_delta = 0.0f;
        _wheels.push_state(MouseWheel::HorizontalWheel, false);
        _wheels.push_state(MouseWheel::VerticalWheel, false);
    }
}

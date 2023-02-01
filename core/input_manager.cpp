#include "input_manager.hpp"

#include <functional>

#include "common.hpp"
#include "event_manager.hpp"

namespace Aporia
{
    InputManager::InputManager(EventManager& event_manager)
    {
        using namespace std::placeholders;

        event_manager.add_listener<BeginProcessingWindowEvents>(std::bind(&InputManager::_reset_wheel, this));

        event_manager.add_listener<KeyPressedEvent>(std::bind(&InputManager::_on_key_triggered, this, _1));
        event_manager.add_listener<KeyReleasedEvent>(std::bind(&InputManager::_on_key_released, this, _1));

        event_manager.add_listener<ButtonPressedEvent>(std::bind(&InputManager::_on_button_triggered, this, _1));
        event_manager.add_listener<ButtonReleasedEvent>(std::bind(&InputManager::_on_button_released, this, _1));

        event_manager.add_listener<MouseWheelScrollEvent>(std::bind(&InputManager::_on_wheel_scrolled, this, _1, _2));

#       if defined(APORIA_EMSCRIPTEN)
            event_manager.add_listener<EndFrameEvent>(std::bind(&InputManager::_update, this));
#       else
            event_manager.add_listener<BeginFrameEvent>(std::bind(&InputManager::_update, this));
#       endif
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

    bool InputManager::is_gamepad_button_triggered(Gamepad gamepad_button) const
    {
        return _gamepad.buttons.is_triggered(gamepad_button);
    }

    bool InputManager::is_gamepad_button_pressed(Gamepad gamepad_button) const
    {
        return _gamepad.buttons.is_pressed(gamepad_button);
    }

    bool InputManager::is_gamepad_button_released(Gamepad gamepad_button) const
    {
        return _gamepad.buttons.is_released(gamepad_button);
    }

    bool InputManager::is_any_gamepad_button_triggered() const
    {
        return _gamepad.buttons.is_any_triggered();
    }

    bool InputManager::is_any_gamepad_button_pressed() const
    {
        return _gamepad.buttons.is_any_pressed();
    }

    bool InputManager::is_any_gamepad_button_released() const
    {
        return _gamepad.buttons.is_any_released();
    }

    f32 InputManager::get_gamepad_axis(GamepadAxis gamepad_axis) const
    {
        const i32 gamepad_axis_index = magic_enum::enum_index(gamepad_axis).value();
        return _gamepad.axes[gamepad_axis_index];
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

    f32 InputManager::is_wheel_scrolling(MouseWheel wheel /* = MouseWheel::VerticalWheel */) const
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

    void InputManager::_on_wheel_scrolled(MouseWheel wheel, f32 delta)
    {
        _wheel_delta = delta;
        _wheels.push_state(wheel, true);
    }

    void InputManager::_reset_wheel()
    {
        _wheel_delta = 0.f;
        _wheels.push_state(MouseWheel::HorizontalWheel, false);
        _wheels.push_state(MouseWheel::VerticalWheel, false);
    }

    void InputManager::_update()
    {
        _keys.update();
        _buttons.update();
        _wheels.update();

        GLFWgamepadstate gamepad_state;
        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad_state))
        {
            for (i32 gamepad_idx = 0; gamepad_idx < magic_enum::enum_count<Gamepad>(); ++gamepad_idx)
            {
                const Gamepad gamepad_code = static_cast<Gamepad>(gamepad_idx);
                const bool new_state = gamepad_state.buttons[gamepad_idx] == GLFW_PRESS;
                _gamepad.buttons.push_state(gamepad_code, new_state);
            }

            for (i32 axis_idx = 0; axis_idx < magic_enum::enum_count<GamepadAxis>(); ++axis_idx)
            {
                _gamepad.axes[axis_idx] = gamepad_state.axes[axis_idx];
            }
        }
    }
}

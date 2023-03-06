#include "input_manager.hpp"

#include "common.hpp"

namespace Aporia
{
    void InputManager::on_key_triggered(Keyboard key)
    {
        _keys.push_state(key, true);
    }

    void InputManager::on_key_released(Keyboard key)
    {
        _keys.push_state(key, false);
    }

    void InputManager::on_button_triggered(Mouse button)
    {
        _mouse.buttons.push_state(button, true);
    }

    void InputManager::on_button_released(Mouse button)
    {
        _mouse.buttons.push_state(button, false);
    }

    void InputManager::on_wheel_scrolled(MouseWheel wheel, f32 delta)
    {
        const u64 wheel_idx = static_cast<u64>(wheel);
        _mouse.wheels[wheel_idx] = delta;
    }

    void InputManager::update()
    {
        _keys.update();
        _mouse.buttons.update();
        _gamepad.buttons.update();

        for (u64 idx = 0; idx < MouseInput::MouseWheelNum; ++idx)
        {
            _mouse.wheels[idx] = 0.f;
        }

        // TODO(dubgron): Update emscripten to use GLFW3.3 (see https://github.com/emscripten-core/emscripten/commit/7cd514d84a1dc7ef966a875400d9c3f1e4ad6c95)
        GLFWgamepadstate gamepad_state;
        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad_state))
        {
            for (u64 gamepad_idx = 0; gamepad_idx < magic_enum::enum_count<Gamepad>(); ++gamepad_idx)
            {
                const Gamepad gamepad_code = static_cast<Gamepad>(gamepad_idx);
                const bool new_state = gamepad_state.buttons[gamepad_idx] == GLFW_PRESS;
                _gamepad.buttons.push_state(gamepad_code, new_state);
            }

            for (u64 axis_idx = 0; axis_idx < magic_enum::enum_count<GamepadAxis>(); ++axis_idx)
            {
                _gamepad.axes[axis_idx] = gamepad_state.axes[axis_idx];
            }
        }
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
        return _mouse.buttons.is_triggered(button);
    }

    bool InputManager::is_button_pressed(Mouse button) const
    {
        return _mouse.buttons.is_pressed(button);
    }

    bool InputManager::is_button_released(Mouse button) const
    {
        return _mouse.buttons.is_released(button);
    }

    bool InputManager::is_any_button_triggered() const
    {
        return _mouse.buttons.is_any_triggered();
    }

    bool InputManager::is_any_button_pressed() const
    {
        return _mouse.buttons.is_any_pressed();
    }

    bool InputManager::is_any_button_released() const
    {
        return _mouse.buttons.is_any_released();
    }

    f32 InputManager::is_wheel_scrolling(MouseWheel wheel /* = MouseWheel::VerticalWheel */) const
    {
        const u64 wheel_idx = static_cast<u64>(wheel);
        return _mouse.wheels[wheel_idx];
    }
}

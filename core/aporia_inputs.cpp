#include "aporia_inputs.hpp"

#include <bitset>

#include <magic_enum.hpp>

namespace Aporia
{
    template<typename T> requires std::is_enum_v<T>
    class InputBuffer
    {
        static constexpr u64 Count = magic_enum::enum_count<T>();
        using Buffer = std::bitset<Count>;

    public:
        void update()
        {
            _past_state = _current_state;
        }

        void push_state(T code, bool state)
        {
            const u64 index = _index(code);
            _current_state[index] = state;
        }

        bool is_triggered(T code) const
        {
            const u64 index = _index(code);
            return _is_triggered(index);
        }

        bool is_pressed(T code) const
        {
            const u64 index = _index(code);
            return _is_pressed(index);
        }

        bool is_released(T code) const
        {
            const u64 index = _index(code);
            return _is_released(index);
        }

        bool is_any_triggered() const
        {
            for (u64 index = 0; index < Count; ++index)
            {
                if (_is_triggered(index))
                {
                    return true;
                }
            }

            return false;
        }

        bool is_any_pressed() const
        {
            for (u64 index = 0; index < Count; ++index)
            {
                if (_is_pressed(index))
                {
                    return true;
                }
            }

            return false;
        }

        bool is_any_released() const
        {
            for (u64 index = 0; index < Count; ++index)
            {
                if (_is_released(index))
                {
                    return true;
                }
            }

            return false;
        }

    private:
        u64 _index(T code) const
        {
            return magic_enum::enum_index(code).value();
        }

        bool _is_triggered(u64 index) const
        {
            return _current_state.test(index) && !_past_state.test(index);
        }

        bool _is_pressed(u64 index) const
        {
            return _current_state.test(index) && _past_state.test(index);
        }

        bool _is_released(u64 index) const
        {
            return !_current_state.test(index) && _past_state.test(index);
        }

        Buffer _current_state;
        Buffer _past_state;
    };

    static constexpr u64 MouseWheelCount = magic_enum::enum_count<MouseWheel>();
    static constexpr u64 GamepadAxisCount = magic_enum::enum_count<GamepadAxis>();

    struct MouseInput
    {
        InputBuffer<Mouse> buttons;
        f32 wheels[MouseWheelCount] = { 0.f };
    };

    struct GamepadInput
    {
        InputBuffer<Gamepad> buttons;
        f32 axes[GamepadAxisCount] = { 0.f };
    };

    static InputBuffer<Keyboard> keyboard;
    static MouseInput mouse;
    static GamepadInput gamepad;

    void inputs_update()
    {
        keyboard.update();
        mouse.buttons.update();
        gamepad.buttons.update();

        for (u64 idx = 0; idx < MouseWheelCount; ++idx)
        {
            mouse.wheels[idx] = 0.f;
        }

        GLFWgamepadstate gamepad_state;
        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad_state))
        {
            for (u64 gamepad_idx = 0; gamepad_idx < magic_enum::enum_count<Gamepad>(); ++gamepad_idx)
            {
                const Gamepad gamepad_code = static_cast<Gamepad>(gamepad_idx);
                const bool new_state = gamepad_state.buttons[gamepad_idx] == GLFW_PRESS;
                gamepad.buttons.push_state(gamepad_code, new_state);
            }

            for (u64 axis_idx = 0; axis_idx < magic_enum::enum_count<GamepadAxis>(); ++axis_idx)
            {
                gamepad.axes[axis_idx] = gamepad_state.axes[axis_idx];
            }
        }
    }

    void on_key_triggered(Keyboard key)
    {
        keyboard.push_state(key, true);
    }

    void on_key_released(Keyboard key)
    {
        keyboard.push_state(key, false);
    }

    void on_button_triggered(Mouse button)
    {
        mouse.buttons.push_state(button, true);
    }

    void on_button_released(Mouse button)
    {
        mouse.buttons.push_state(button, false);
    }

    void on_wheel_scrolled(MouseWheel wheel, f32 delta)
    {
        const u64 wheel_idx = static_cast<u64>(wheel);
        mouse.wheels[wheel_idx] = delta;
    }

    bool is_key_triggered(Keyboard key)
    {
        return keyboard.is_triggered(key);
    }

    bool is_key_pressed(Keyboard key)
    {
        return keyboard.is_pressed(key);
    }

    bool is_key_released(Keyboard key)
    {
        return keyboard.is_released(key);
    }

    bool is_any_key_triggered()
    {
        return keyboard.is_any_triggered();
    }

    bool is_any_key_pressed()
    {
        return keyboard.is_any_pressed();
    }

    bool is_any_key_released()
    {
        return keyboard.is_any_released();
    }

    bool is_gamepad_button_triggered(Gamepad gamepad_button)
    {
        return gamepad.buttons.is_triggered(gamepad_button);
    }

    bool is_gamepad_button_pressed(Gamepad gamepad_button)
    {
        return gamepad.buttons.is_pressed(gamepad_button);
    }

    bool is_gamepad_button_released(Gamepad gamepad_button)
    {
        return gamepad.buttons.is_released(gamepad_button);
    }

    bool is_any_gamepad_button_triggered()
    {
        return gamepad.buttons.is_any_triggered();
    }

    bool is_any_gamepad_button_pressed()
    {
        return gamepad.buttons.is_any_pressed();
    }

    bool is_any_gamepad_button_released()
    {
        return gamepad.buttons.is_any_released();
    }

    f32 get_gamepad_axis(GamepadAxis gamepad_axis)
    {
        const u64 gamepad_axis_index = magic_enum::enum_index(gamepad_axis).value();
        return gamepad.axes[gamepad_axis_index];
    }

    bool is_button_triggered(Mouse button)
    {
        return mouse.buttons.is_triggered(button);
    }

    bool is_button_pressed(Mouse button)
    {
        return mouse.buttons.is_pressed(button);
    }

    bool is_button_released(Mouse button)
    {
        return mouse.buttons.is_released(button);
    }

    bool is_any_button_triggered()
    {
        return mouse.buttons.is_any_triggered();
    }

    bool is_any_button_pressed()
    {
        return mouse.buttons.is_any_pressed();
    }

    bool is_any_button_released()
    {
        return mouse.buttons.is_any_released();
    }

    f32 is_wheel_scrolling(MouseWheel wheel /* = MouseWheel::VerticalWheel */)
    {
        const u64 wheel_idx = static_cast<u64>(wheel);
        return mouse.wheels[wheel_idx];
    }
}

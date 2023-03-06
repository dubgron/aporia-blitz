#pragma once

#include "input_buffer.hpp"
#include "inputs/gamepad.hpp"
#include "inputs/keyboard.hpp"
#include "inputs/mouse.hpp"

#include "aporia_types.hpp"

namespace Aporia
{
    struct GamepadInput
    {
        static constexpr u64 AxisNum = magic_enum::enum_count<GamepadAxis>();

        InputBuffer<Gamepad> buttons;
        f32 axes[AxisNum] = { 0.f };
    };

    struct MouseInput
    {
        static constexpr u64 MouseWheelNum = magic_enum::enum_count<MouseWheel>();

        InputBuffer<Mouse> buttons;
        f32 wheels[MouseWheelNum] = { 0.f };
    };

    class InputManager final
    {
    public:
        void on_key_triggered(Keyboard key);
        void on_key_released(Keyboard key);

        void on_button_triggered(Mouse button);
        void on_button_released(Mouse button);

        void on_wheel_scrolled(MouseWheel wheel, f32 delta);

        void update();

        /* Keyboard */
        bool is_key_triggered(Keyboard key) const;
        bool is_key_pressed(Keyboard key) const;
        bool is_key_released(Keyboard key) const;

        bool is_any_key_triggered() const;
        bool is_any_key_pressed() const;
        bool is_any_key_released() const;

        /* Gamepad */
        bool is_gamepad_button_triggered(Gamepad gamepad_button) const;
        bool is_gamepad_button_pressed(Gamepad gamepad_button) const;
        bool is_gamepad_button_released(Gamepad gamepad_button) const;

        bool is_any_gamepad_button_triggered() const;
        bool is_any_gamepad_button_pressed() const;
        bool is_any_gamepad_button_released() const;

        f32 get_gamepad_axis(GamepadAxis gamepad_axis) const;

        /* Mouse */
        bool is_button_triggered(Mouse button) const;
        bool is_button_pressed(Mouse button) const;
        bool is_button_released(Mouse button) const;

        bool is_any_button_triggered() const;
        bool is_any_button_pressed() const;
        bool is_any_button_released() const;

        f32 is_wheel_scrolling(MouseWheel wheel = MouseWheel::VerticalWheel) const;

    private:
        InputBuffer<Keyboard> _keys;
        GamepadInput _gamepad;
        MouseInput _mouse;
    };
}

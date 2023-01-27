#pragma once

#include "input_buffer.hpp"
#include "inputs/gamepad.hpp"
#include "inputs/keyboard.hpp"
#include "inputs/mouse.hpp"

namespace Aporia
{
    class EventManager;

    struct GamepadInput final
    {
        static constexpr size_t AxisNum = magic_enum::enum_count<GamepadAxis>();

        InputBuffer<Gamepad> buttons;
        std::array<float, AxisNum> axes = { 0.0f };
    };

    class InputManager final
    {
    public:
        InputManager(EventManager& event_manager);

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

        float get_gamepad_axis(GamepadAxis gamepad_axis) const;

        /* Mouse */
        bool is_button_triggered(Mouse button) const;
        bool is_button_pressed(Mouse button) const;
        bool is_button_released(Mouse button) const;

        bool is_any_button_triggered() const;
        bool is_any_button_pressed() const;
        bool is_any_button_released() const;

        float is_wheel_scrolling(MouseWheel wheel = MouseWheel::VerticalWheel) const;

    private:
        void _on_key_triggered(Keyboard key);
        void _on_key_released(Keyboard key);

        void _on_button_triggered(Mouse button);
        void _on_button_released(Mouse button);

        void _on_wheel_scrolled(MouseWheel wheel, float delta);
        void _reset_wheel();

        void _update();

    private:
        InputBuffer<Keyboard> _keys;
        GamepadInput _gamepad;
        InputBuffer<Mouse> _buttons;
        InputBuffer<MouseWheel> _wheels;
        float _wheel_delta = 0.0f;
    };
}

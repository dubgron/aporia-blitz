#pragma once 

#include <array>
#include <memory>

#include "event.hpp"
#include "event_manager.hpp"
#include "input_buffer.hpp"
#include "inputs.hpp"
#include "logger.hpp"
#include "platform.hpp"

namespace Aporia
{
    class APORIA_API InputManager final
    {
    public:
        InputManager(Logger& logger, EventManager& event_manager);

        bool is_key_triggered(Keyboard key) const;
        bool is_key_pressed(Keyboard key) const;
        bool is_key_released(Keyboard key) const;

        bool is_any_key_triggered() const;
        bool is_any_key_pressed() const;
        bool is_any_key_released() const;

        bool is_button_triggered(Mouse button) const;
        bool is_button_pressed(Mouse button) const;
        bool is_button_released(Mouse button) const;

        bool is_any_button_triggered() const;
        bool is_any_button_pressed() const;
        bool is_any_button_released() const;

        float is_wheel_scrolling(MouseWheel wheel = MouseWheel::VerticalWheel) const;

        sf::Vector2i get_mouse_position() const;

    private:
        void _on_key_triggered(Keyboard key);
        void _on_key_released(Keyboard key);

        void _on_button_triggered(Mouse button);
        void _on_button_released(Mouse button);

        void _on_wheel_scrolled(MouseWheel wheel, float delta);
        void _reset_wheel();

        InputBuffer<Keyboard> _keys;
        InputBuffer<Mouse> _buttons;
        InputBuffer<MouseWheel> _wheels;
        float _wheel_delta = 0.0f;

        Logger& _logger;
    };
}

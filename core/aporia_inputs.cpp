#include "aporia_inputs.hpp"

#include "aporia_debug.hpp"
#include "aporia_utils.hpp"

#if defined(APORIA_EMSCRIPTEN)
    int glfwGetGamepadState(int jid, GLFWgamepadstate* state) { return 0; }
    int glfwGetError(const char** description) { return 0; }
#endif

namespace Aporia
{
    Input input;

    enum Flags : u8
    {
        none                = 0x00,
        ended_frame_down    = 0x01,
        was_released        = 0x02,
        is_repeated         = 0x04,
        was_handled         = 0x08
    };

    static InputState get_input_state(Key key)
    {
        u64 key_code = +key;
        APORIA_ASSERT(key_code < +Key::Count);
        return input.keys[key_code];
    }

    static InputState get_input_state(MouseButton button)
    {
        const u64 button_code = +button;
        APORIA_ASSERT(button_code < +MouseButton::Count);
        return input.mouse[button_code];
    }

    static InputState get_input_state(GamepadButton button)
    {
        const u64 button_code = +button;
        APORIA_ASSERT(button_code < +GamepadButton::Count);
        return input.buttons[button_code];
    }

    static void clear_input_state(InputState* state, u64 count)
    {
        for (u64 idx = 0; idx < count; ++idx)
        {
            // Leave only InputFlag::ended_frame_down flag
            state[idx].flags &= ended_frame_down;
            state[idx].pressed_count = 0;
        }
    }

    static i32 has_been_pressed(InputState state)
    {
        return (state.flags & was_handled) ? 0 : state.pressed_count;
    }

    static bool has_been_held(InputState state)
    {
        return !(state.flags & was_handled) && (state.flags & ended_frame_down);
    }

    static bool has_been_released(InputState state)
    {
        return !(state.flags & was_handled) && (state.flags & was_released);
    }

    void process_input_action(InputState& state, InputAction action)
    {
        switch (action)
        {
            case InputAction::Released:
            {
                state.flags &= ~ended_frame_down;
                state.flags |= was_released;
            }
            break;

            case InputAction::Pressed:
            {
                state.pressed_count += 1;
                state.flags |= ended_frame_down;
            }
            break;

            case InputAction::Repeat:
            {
                state.flags |= is_repeated;
            }
            break;
        }
    }

    void process_input_value(AnalogInputState& state, f32 value)
    {
        state.end_value = value;
        state.max_value = max<f32>(state.max_value, value);
        state.min_value = min<f32>(state.min_value, value);
    }

    void poll_gamepad_inputs()
    {
#if !defined(APORIA_EMSCRIPTEN)
        GLFWgamepadstate gamepad_state;
        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad_state))
        {
            for (u64 gamepad_idx = 0; gamepad_idx < +GamepadButton::Count; ++gamepad_idx)
            {
                const InputAction action = static_cast<InputAction>(gamepad_state.buttons[gamepad_idx]);
                process_input_action(input.buttons[gamepad_idx], action);
            }

            for (u64 axis_idx = 0; axis_idx < +GamepadAxis::Count; ++axis_idx)
            {
                const f32 current_value = gamepad_state.axes[axis_idx];
                process_input_value(input.axes[axis_idx], current_value);
            }
        }
#endif
    }

    void inputs_clear()
    {
        clear_input_state(input.keys, +Key::Count);
        clear_input_state(input.mouse, +MouseButton::Count);
        clear_input_state(input.buttons, +GamepadButton::Count);

        memset(input.wheels, 0.f, sizeof(AnalogInputState) * +MouseWheel::Count);
        memset(input.axes, 0.f, sizeof(AnalogInputState) * +GamepadAxis::Count);
    }

    i32 has_been_pressed(Key key)
    {
        const InputState state = get_input_state(key);
        return has_been_pressed(state);
    }

    bool has_been_held(Key key)
    {
        const InputState state = get_input_state(key);
        return has_been_held(state);
    }

    bool has_been_released(Key key)
    {
        const InputState state = get_input_state(key);
        return has_been_released(state);
    }

    i32 has_been_pressed(MouseButton button)
    {
        const InputState state = get_input_state(button);
        return has_been_pressed(state);
    }

    bool has_been_held(MouseButton button)
    {
        const InputState state = get_input_state(button);
        return has_been_held(state);
    }

    bool has_been_released(MouseButton button)
    {
        const InputState state = get_input_state(button);
        return has_been_released(state);
    }

    i32 has_been_pressed(GamepadButton button)
    {
        const InputState state = get_input_state(button);
        return has_been_pressed(state);
    }

    bool has_been_held(GamepadButton button)
    {
        const InputState state = get_input_state(button);
        return has_been_held(state);
    }

    bool has_been_released(GamepadButton button)
    {
        const InputState state = get_input_state(button);
        return has_been_released(state);
    }

    AnalogInputState get_analog_state(MouseWheel wheel)
    {
        const u64 wheel_code = +wheel;
        APORIA_ASSERT(wheel_code < +GamepadAxis::Count);
        return input.wheels[wheel_code];
    }

    AnalogInputState get_analog_state(GamepadAxis axis)
    {
        const u64 axis_code = +axis;
        APORIA_ASSERT(axis_code < +GamepadAxis::Count);
        return input.axes[axis_code];
    }

    Key string_to_key(std::string_view string)
    {
        if (string == "Unknown")        return Key::Unknown;
        if (string == "Space")          return Key::Space;
        if (string == "Quote")          return Key::Quote;
        if (string == "Comma")          return Key::Comma;
        if (string == "Minus")          return Key::Minus;
        if (string == "Period")         return Key::Period;
        if (string == "Slash")          return Key::Slash;
        if (string == "Num0")           return Key::Num0;
        if (string == "Num1")           return Key::Num1;
        if (string == "Num2")           return Key::Num2;
        if (string == "Num3")           return Key::Num3;
        if (string == "Num4")           return Key::Num4;
        if (string == "Num5")           return Key::Num5;
        if (string == "Num6")           return Key::Num6;
        if (string == "Num7")           return Key::Num7;
        if (string == "Num8")           return Key::Num8;
        if (string == "Num9")           return Key::Num9;
        if (string == "Semicolon")      return Key::Semicolon;
        if (string == "Equal")          return Key::Equal;
        if (string == "A")              return Key::A;
        if (string == "B")              return Key::B;
        if (string == "C")              return Key::C;
        if (string == "D")              return Key::D;
        if (string == "E")              return Key::E;
        if (string == "F")              return Key::F;
        if (string == "G")              return Key::G;
        if (string == "H")              return Key::H;
        if (string == "I")              return Key::I;
        if (string == "J")              return Key::J;
        if (string == "K")              return Key::K;
        if (string == "L")              return Key::L;
        if (string == "M")              return Key::M;
        if (string == "N")              return Key::N;
        if (string == "O")              return Key::O;
        if (string == "P")              return Key::P;
        if (string == "Q")              return Key::Q;
        if (string == "R")              return Key::R;
        if (string == "S")              return Key::S;
        if (string == "T")              return Key::T;
        if (string == "U")              return Key::U;
        if (string == "V")              return Key::V;
        if (string == "W")              return Key::W;
        if (string == "X")              return Key::X;
        if (string == "Y")              return Key::Y;
        if (string == "Z")              return Key::Z;
        if (string == "LBracket")       return Key::LBracket;
        if (string == "Backslash")      return Key::Backslash;
        if (string == "RBracket")       return Key::RBracket;
        if (string == "Tilde")          return Key::Tilde;
        if (string == "World1")         return Key::World1;
        if (string == "World2")         return Key::World2;
        if (string == "Escape")         return Key::Escape;
        if (string == "Enter")          return Key::Enter;
        if (string == "Tab")            return Key::Tab;
        if (string == "Backspace")      return Key::Backspace;
        if (string == "Insert")         return Key::Insert;
        if (string == "Delete")         return Key::Delete;
        if (string == "Right")          return Key::Right;
        if (string == "Left")           return Key::Left;
        if (string == "Down")           return Key::Down;
        if (string == "Up")             return Key::Up;
        if (string == "PageUp")         return Key::PageUp;
        if (string == "PageDown")       return Key::PageDown;
        if (string == "Home")           return Key::Home;
        if (string == "End")            return Key::End;
        if (string == "CapsLock")       return Key::CapsLock;
        if (string == "ScrollLock")     return Key::ScrollLock;
        if (string == "NumLock")        return Key::NumLock;
        if (string == "PrintScreen")    return Key::PrintScreen;
        if (string == "Pause")          return Key::Pause;
        if (string == "F1")             return Key::F1;
        if (string == "F2")             return Key::F2;
        if (string == "F3")             return Key::F3;
        if (string == "F4")             return Key::F4;
        if (string == "F5")             return Key::F5;
        if (string == "F6")             return Key::F6;
        if (string == "F7")             return Key::F7;
        if (string == "F8")             return Key::F8;
        if (string == "F9")             return Key::F9;
        if (string == "F10")            return Key::F10;
        if (string == "F11")            return Key::F11;
        if (string == "F12")            return Key::F12;
        if (string == "F13")            return Key::F13;
        if (string == "F14")            return Key::F14;
        if (string == "F15")            return Key::F15;
        if (string == "F16")            return Key::F16;
        if (string == "F17")            return Key::F17;
        if (string == "F18")            return Key::F18;
        if (string == "F19")            return Key::F19;
        if (string == "F20")            return Key::F20;
        if (string == "F21")            return Key::F21;
        if (string == "F22")            return Key::F22;
        if (string == "F23")            return Key::F23;
        if (string == "F24")            return Key::F24;
        if (string == "F25")            return Key::F25;
        if (string == "Numpad0")        return Key::Numpad0;
        if (string == "Numpad1")        return Key::Numpad1;
        if (string == "Numpad2")        return Key::Numpad2;
        if (string == "Numpad3")        return Key::Numpad3;
        if (string == "Numpad4")        return Key::Numpad4;
        if (string == "Numpad5")        return Key::Numpad5;
        if (string == "Numpad6")        return Key::Numpad6;
        if (string == "Numpad7")        return Key::Numpad7;
        if (string == "Numpad8")        return Key::Numpad8;
        if (string == "Numpad9")        return Key::Numpad9;
        if (string == "NumpadPeriod")   return Key::NumpadPeriod;
        if (string == "NumpadDivide")   return Key::NumpadDivide;
        if (string == "NumpadMultiply") return Key::NumpadMultiply;
        if (string == "NumpadSubtract") return Key::NumpadSubtract;
        if (string == "NumpadAdd")      return Key::NumpadAdd;
        if (string == "NumpadEnter")    return Key::NumpadEnter;
        if (string == "NumpadEqual")    return Key::NumpadEqual;
        if (string == "LShift")         return Key::LShift;
        if (string == "LControl")       return Key::LControl;
        if (string == "LAlt")           return Key::LAlt;
        if (string == "LSuper")         return Key::LSuper;
        if (string == "RShift")         return Key::RShift;
        if (string == "RControl")       return Key::RControl;
        if (string == "RAlt")           return Key::RAlt;
        if (string == "RSuper")         return Key::RSuper;
        if (string == "Menu")           return Key::Menu;

        return Key::Unknown;
    }
}

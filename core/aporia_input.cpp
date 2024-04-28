#include "aporia_input.hpp"

#include "aporia_debug.hpp"
#include "aporia_utils.hpp"

#if defined(APORIA_EDITOR)
#include "editor/aporia_editor.hpp"
#endif

#if defined(APORIA_EMSCRIPTEN)
    int glfwGetGamepadState(int jid, GLFWgamepadstate* state) { return 0; }
    int glfwGetError(const char** description) { return 0; }
#endif

static Input input;

// @TODO(dubgron): This is weird, we should figure something nicer.
static InputOwner active_owner = InputOwner_None;

static void input_process_event(InputState* state, InputAction action)
{
    switch (action)
    {
        case InputAction_Released:
        {
            if (state->flags & InputFlag_EndedFrameDown)
            {
                // Unset InputFlag_EndedFrameDown flag.
                state->flags &= ~InputFlag_EndedFrameDown;
                state->flags |= InputFlag_WasReleased;
            }
        }
        break;

        case InputAction_Pressed:
        {
            state->pressed_count += 1;
            state->flags |= InputFlag_EndedFrameDown;
        }
        break;

        case InputAction_Repeat:
        {
            state->flags |= InputFlag_IsRepeated;
        }
        break;
    }
}

static void input_process_event(AnalogInputState* state, f32 value)
{
    state->end_value = value;
    state->max_value = max<f32>(state->max_value, value);
    state->min_value = min<f32>(state->min_value, value);
}

void input_process_key_event(Key key, InputAction action)
{
    if (key != Key_Unknown)
    {
        input_process_event(&input.keys[key], action);
    }
    else switch (action)
    {
        case InputAction_Released: APORIA_LOG(Warning, "The unknown key has been released!");  break;
        case InputAction_Pressed:  APORIA_LOG(Warning, "The unknown key has been pressed!");   break;
        case InputAction_Repeat:   APORIA_LOG(Warning, "The unknown key has been held!");      break;
    }
}

void input_process_mouse_event(MouseButton button, InputAction action)
{
    input_process_event(&input.mouse[button], action);
}

void input_process_gamepad_event(GamepadButton button, InputAction action)
{
    input_process_event(&input.gamepad[button], action);
}

void input_process_scroll_event(MouseWheel wheel, f32 value)
{
    input_process_event(&input.wheels[wheel], value);
}

void input_process_analog_event(GamepadAnalog analog, f32 value)
{
    input_process_event(&input.analogs[analog], value);
}

void input_clear()
{
    constexpr auto input_release_if_owner_changed = []<u64 N>(InputState (&state)[N])
    {
        for (u64 idx = 0; idx < N; ++idx)
        {
            if (state[idx].owner != InputOwner_None && state[idx].owner != state[idx].last_owner)
            {
                input_process_event(&state[idx], InputAction_Released);
            }
        }
    };

    input_release_if_owner_changed(input.keys);
    input_release_if_owner_changed(input.mouse);
    input_release_if_owner_changed(input.gamepad);

    constexpr auto input_prepare_for_next_frame = []<u64 N>(InputState (&state)[N])
    {
        for (u64 idx = 0; idx < N; ++idx)
        {
            // Leave only InputFlag_EndedFrameDown flag.
            state[idx].flags &= InputFlag_EndedFrameDown;
            state[idx].pressed_count = 0;

            state[idx].last_owner = state[idx].owner;
            state[idx].owner = InputOwner_None;
        }
    };

    input_prepare_for_next_frame(input.keys);
    input_prepare_for_next_frame(input.mouse);
    input_prepare_for_next_frame(input.gamepad);

    ARRAY_ZERO(input.wheels);
    ARRAY_ZERO(input.analogs);
}

void input_process_events()
{
    GLFWgamepadstate gamepad_state;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad_state))
    {
        for (u64 gamepad_idx = 0; gamepad_idx < ARRAY_COUNT(input.gamepad); ++gamepad_idx)
        {
            GamepadButton button = (GamepadButton)gamepad_idx;
            InputAction action = (InputAction)gamepad_state.buttons[gamepad_idx];
            input_process_gamepad_event(button, action);
        }

        for (u64 analog_idx = 0; analog_idx < ARRAY_COUNT(input.analogs); ++analog_idx)
        {
            GamepadAnalog analog = (GamepadAnalog)analog_idx;
            f32 current_value = gamepad_state.axes[analog_idx];
            input_process_analog_event(analog, current_value);
        }
    }

#if defined(APORIA_IMGUI)

    // @NOTE(dubgron): According to ImGui comments:
    //  - WantCaptureKeyboard means "InputText active, or an imgui window is focused
    //      and navigation is enabled, etc."
    //  - WantCaptureMouse means "unclicked mouse is hovering over an imgui window,
    //      widget is active, mouse was clicked over an imgui window, etc."
    //  - WantTextInput means "InputText widget is active."
    ImGuiIO* io = &ImGui::GetIO();

    constexpr auto input_set_owner = []<u64 N>(InputState(&state)[N], InputOwner owner)
    {
        for (u64 idx = 0; idx < N; ++idx)
        {
            state[idx].owner = owner;
        }
    };

#if defined(APORIA_EDITOR)
    if (editor_is_open && !io->WantTextInput)
    {
        input_set_owner(input.keys, InputOwner_Editor);
        input_set_owner(input.gamepad, InputOwner_Editor);
    }
    else
#endif
    if (io->WantCaptureKeyboard)
    {
        input_set_owner(input.keys, InputOwner_ImGui);
        input_set_owner(input.gamepad, InputOwner_ImGui);
    }

#if defined(APORIA_EDITOR)
    input_set_owner(input.mouse, InputOwner_Editor);
#else
    if (io->WantCaptureMouse)
    {
        input_set_owner(input.mouse, InputOwner_ImGui);
    }
#endif

#endif
}

void input_set_active_owner(InputOwner owner)
{
    active_owner = owner;
}

static InputState input_resolve_ownership(InputState* state)
{
    if (state->owner == InputOwner_None || state->owner == active_owner)
    {
        state->owner = active_owner;
        return *state;
    }
    else if (state->last_owner == active_owner)
    {
        InputState result;
        if (state->flags & InputFlag_EndedFrameDown)
        {
            result.flags |= InputFlag_WasReleased;
        }
        return result;
    }
    else
    {
        return InputState{};
    }
}

InputState input_get(Key key)
{
    InputState* state = &input.keys[key];
    return input_resolve_ownership(state);
}

InputState input_get(MouseButton button)
{
    InputState* state = &input.mouse[button];
    return input_resolve_ownership(state);
}

InputState input_get(GamepadButton button)
{
    InputState* state = &input.gamepad[button];
    return input_resolve_ownership(state);
}

AnalogInputState input_get(MouseWheel wheel)
{
    return input.wheels[wheel];
}

AnalogInputState input_get(GamepadAnalog analog)
{
    return input.analogs[analog];
}

bool input_is_pressed(InputState state)
{
    return state.pressed_count > 0;
}

bool input_is_held(InputState state)
{
    return (state.flags & InputFlag_EndedFrameDown);
}

bool input_is_released(InputState state)
{
    return (state.flags & InputFlag_WasReleased);
}

bool input_is_repeated(InputState state)
{
    return (state.flags & InputFlag_IsRepeated);
}

bool input_is_pressed(Key key)
{
    InputState state = input_get(key);
    return input_is_pressed(state);
}

bool input_is_held(Key key)
{
    InputState state = input_get(key);
    return input_is_held(state);
}

bool input_is_released(Key key)
{
    InputState state = input_get(key);
    return input_is_released(state);
}

bool input_is_repeated(Key key)
{
    InputState state = input_get(key);
    return input_is_repeated(state);
}

bool input_is_pressed(MouseButton button)
{
    InputState state = input_get(button);
    return input_is_pressed(state);
}

bool input_is_held(MouseButton button)
{
    InputState state = input_get(button);
    return input_is_held(state);
}

bool input_is_released(MouseButton button)
{
    InputState state = input_get(button);
    return input_is_released(state);
}

bool input_is_repeated(MouseButton button)
{
    InputState state = input_get(button);
    return input_is_repeated(state);
}

bool input_is_pressed(GamepadButton button)
{
    InputState state = input_get(button);
    return input_is_pressed(state);
}

bool input_is_held(GamepadButton button)
{
    InputState state = input_get(button);
    return input_is_held(state);
}

bool input_is_released(GamepadButton button)
{
    InputState state = input_get(button);
    return input_is_released(state);
}

bool input_is_repeated(GamepadButton button)
{
    InputState state = input_get(button);
    return input_is_repeated(state);
}

bool input_is_any_key_pressed()
{
    bool result = false;
    for (u64 key_idx = 0; key_idx < ARRAY_COUNT(input.keys); ++key_idx)
    {
        Key key = (Key)key_idx;
        result |= input_is_pressed(key);
    }
    return result;
}

bool input_is_any_mouse_button_pressed()
{
    bool result = false;
    for (u64 mouse_idx = 0; mouse_idx < ARRAY_COUNT(input.mouse); ++mouse_idx)
    {
        MouseButton button = (MouseButton)mouse_idx;
        result |= input_is_pressed(button);
    }
    return result;
}

bool input_is_any_gamepad_button_pressed()
{
    bool result = false;
    for (u64 gamepad_idx = 0; gamepad_idx < ARRAY_COUNT(input.gamepad); ++gamepad_idx)
    {
        GamepadButton button = (GamepadButton)gamepad_idx;
        result |= input_is_pressed(button);
    }
    return result;
}

Key string_to_key(String string)
{
    if (string == "Unknown")        return Key_Unknown;
    if (string == "Space")          return Key_Space;
    if (string == "Quote")          return Key_Quote;
    if (string == "Comma")          return Key_Comma;
    if (string == "Minus")          return Key_Minus;
    if (string == "Period")         return Key_Period;
    if (string == "Slash")          return Key_Slash;
    if (string == "Num0")           return Key_Num0;
    if (string == "Num1")           return Key_Num1;
    if (string == "Num2")           return Key_Num2;
    if (string == "Num3")           return Key_Num3;
    if (string == "Num4")           return Key_Num4;
    if (string == "Num5")           return Key_Num5;
    if (string == "Num6")           return Key_Num6;
    if (string == "Num7")           return Key_Num7;
    if (string == "Num8")           return Key_Num8;
    if (string == "Num9")           return Key_Num9;
    if (string == "Semicolon")      return Key_Semicolon;
    if (string == "Equal")          return Key_Equal;
    if (string == "A")              return Key_A;
    if (string == "B")              return Key_B;
    if (string == "C")              return Key_C;
    if (string == "D")              return Key_D;
    if (string == "E")              return Key_E;
    if (string == "F")              return Key_F;
    if (string == "G")              return Key_G;
    if (string == "H")              return Key_H;
    if (string == "I")              return Key_I;
    if (string == "J")              return Key_J;
    if (string == "K")              return Key_K;
    if (string == "L")              return Key_L;
    if (string == "M")              return Key_M;
    if (string == "N")              return Key_N;
    if (string == "O")              return Key_O;
    if (string == "P")              return Key_P;
    if (string == "Q")              return Key_Q;
    if (string == "R")              return Key_R;
    if (string == "S")              return Key_S;
    if (string == "T")              return Key_T;
    if (string == "U")              return Key_U;
    if (string == "V")              return Key_V;
    if (string == "W")              return Key_W;
    if (string == "X")              return Key_X;
    if (string == "Y")              return Key_Y;
    if (string == "Z")              return Key_Z;
    if (string == "LBracket")       return Key_LBracket;
    if (string == "Backslash")      return Key_Backslash;
    if (string == "RBracket")       return Key_RBracket;
    if (string == "Tilde")          return Key_Tilde;
    if (string == "World1")         return Key_World1;
    if (string == "World2")         return Key_World2;
    if (string == "Escape")         return Key_Escape;
    if (string == "Enter")          return Key_Enter;
    if (string == "Tab")            return Key_Tab;
    if (string == "Backspace")      return Key_Backspace;
    if (string == "Insert")         return Key_Insert;
    if (string == "Delete")         return Key_Delete;
    if (string == "Right")          return Key_Right;
    if (string == "Left")           return Key_Left;
    if (string == "Down")           return Key_Down;
    if (string == "Up")             return Key_Up;
    if (string == "PageUp")         return Key_PageUp;
    if (string == "PageDown")       return Key_PageDown;
    if (string == "Home")           return Key_Home;
    if (string == "End")            return Key_End;
    if (string == "CapsLock")       return Key_CapsLock;
    if (string == "ScrollLock")     return Key_ScrollLock;
    if (string == "NumLock")        return Key_NumLock;
    if (string == "PrintScreen")    return Key_PrintScreen;
    if (string == "Pause")          return Key_Pause;
    if (string == "F1")             return Key_F1;
    if (string == "F2")             return Key_F2;
    if (string == "F3")             return Key_F3;
    if (string == "F4")             return Key_F4;
    if (string == "F5")             return Key_F5;
    if (string == "F6")             return Key_F6;
    if (string == "F7")             return Key_F7;
    if (string == "F8")             return Key_F8;
    if (string == "F9")             return Key_F9;
    if (string == "F10")            return Key_F10;
    if (string == "F11")            return Key_F11;
    if (string == "F12")            return Key_F12;
    if (string == "F13")            return Key_F13;
    if (string == "F14")            return Key_F14;
    if (string == "F15")            return Key_F15;
    if (string == "F16")            return Key_F16;
    if (string == "F17")            return Key_F17;
    if (string == "F18")            return Key_F18;
    if (string == "F19")            return Key_F19;
    if (string == "F20")            return Key_F20;
    if (string == "F21")            return Key_F21;
    if (string == "F22")            return Key_F22;
    if (string == "F23")            return Key_F23;
    if (string == "F24")            return Key_F24;
    if (string == "F25")            return Key_F25;
    if (string == "Numpad0")        return Key_Numpad0;
    if (string == "Numpad1")        return Key_Numpad1;
    if (string == "Numpad2")        return Key_Numpad2;
    if (string == "Numpad3")        return Key_Numpad3;
    if (string == "Numpad4")        return Key_Numpad4;
    if (string == "Numpad5")        return Key_Numpad5;
    if (string == "Numpad6")        return Key_Numpad6;
    if (string == "Numpad7")        return Key_Numpad7;
    if (string == "Numpad8")        return Key_Numpad8;
    if (string == "Numpad9")        return Key_Numpad9;
    if (string == "NumpadPeriod")   return Key_NumpadPeriod;
    if (string == "NumpadDivide")   return Key_NumpadDivide;
    if (string == "NumpadMultiply") return Key_NumpadMultiply;
    if (string == "NumpadSubtract") return Key_NumpadSubtract;
    if (string == "NumpadAdd")      return Key_NumpadAdd;
    if (string == "NumpadEnter")    return Key_NumpadEnter;
    if (string == "NumpadEqual")    return Key_NumpadEqual;
    if (string == "LShift")         return Key_LShift;
    if (string == "LControl")       return Key_LControl;
    if (string == "LAlt")           return Key_LAlt;
    if (string == "LSuper")         return Key_LSuper;
    if (string == "RShift")         return Key_RShift;
    if (string == "RControl")       return Key_RControl;
    if (string == "RAlt")           return Key_RAlt;
    if (string == "RSuper")         return Key_RSuper;
    if (string == "Menu")           return Key_Menu;

    return Key_Unknown;
}

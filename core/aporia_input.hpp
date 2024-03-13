#pragma once

#include "aporia_string.hpp"
#include "aporia_types.hpp"

#define DEFINE_ENUM_TO_U64_CAST(enum_name) \
    inline constexpr u64 operator+(enum_name value) { return static_cast<u64>(value); }

namespace Aporia
{
    enum class Key : i32
    {
        Unknown             = -1,   // Unhandled key
        Space               = 32,   // The Space key
        Quote               = 39,   // The ' key
        Comma               = 44,   // The , key
        Minus               = 45,   // The - key
        Period              = 46,   // The . key
        Slash               = 47,   // The / key
        Num0                = 48,   // The 0 key
        Num1                = 49,   // The 1 key
        Num2                = 50,   // The 2 key
        Num3                = 51,   // The 3 key
        Num4                = 52,   // The 4 key
        Num5                = 53,   // The 5 key
        Num6                = 54,   // The 6 key
        Num7                = 55,   // The 7 key
        Num8                = 56,   // The 8 key
        Num9                = 57,   // The 9 key
        Semicolon           = 59,   // The ; key
        Equal               = 61,   // The = key
        A                   = 65,   // The A key
        B                   = 66,   // The B key
        C                   = 67,   // The C key
        D                   = 68,   // The D key
        E                   = 69,   // The E key
        F                   = 70,   // The F key
        G                   = 71,   // The G key
        H                   = 72,   // The H key
        I                   = 73,   // The I key
        J                   = 74,   // The J key
        K                   = 75,   // The K key
        L                   = 76,   // The L key
        M                   = 77,   // The M key
        N                   = 78,   // The N key
        O                   = 79,   // The O key
        P                   = 80,   // The P key
        Q                   = 81,   // The Q key
        R                   = 82,   // The R key
        S                   = 83,   // The S key
        T                   = 84,   // The T key
        U                   = 85,   // The U key
        V                   = 86,   // The V key
        W                   = 87,   // The W key
        X                   = 88,   // The X key
        Y                   = 89,   // The Y key
        Z                   = 90,   // The Z key
        LBracket            = 91,   // The ( key
        Backslash           = 92,   // The \ key
        RBracket            = 93,   // The ) key
        Tilde               = 96,   // The ~ key
        World1              = 161,  // The World1 key
        World2              = 162,  // The World2 key
        Escape              = 256,  // The Escape key
        Enter               = 257,  // The Enter/Return keys
        Tab                 = 258,  // The Tab key
        Backspace           = 259,  // The Backspace key
        Insert              = 260,  // The Insert key
        Delete              = 261,  // The Delete key
        Right               = 262,  // The Right key
        Left                = 263,  // The Left key
        Down                = 264,  // The Down key
        Up                  = 265,  // The Up key
        PageUp              = 266,  // The PageUp key
        PageDown            = 267,  // The PageDown key
        Home                = 268,  // The Home key
        End                 = 279,  // The End key
        CapsLock            = 280,  // The CapsLock key
        ScrollLock          = 281,  // The ScrollLock key
        NumLock             = 282,  // The NumLock key
        PrintScreen         = 283,  // The PrintScreen key
        Pause               = 284,  // The Pause key
        F1                  = 290,  // The F1 key
        F2                  = 291,  // The F2 key
        F3                  = 292,  // The F3 key
        F4                  = 293,  // The F4 key
        F5                  = 294,  // The F5 key
        F6                  = 295,  // The F6 key
        F7                  = 296,  // The F7 key
        F8                  = 297,  // The F8 key
        F9                  = 298,  // The F9 key
        F10                 = 299,  // The F10 key
        F11                 = 300,  // The F11 key
        F12                 = 301,  // The F12 key
        F13                 = 302,  // The F13 key
        F14                 = 303,  // The F14 key
        F15                 = 304,  // The F15 key
        F16                 = 305,  // The F16 key
        F17                 = 306,  // The F17 key
        F18                 = 307,  // The F18 key
        F19                 = 308,  // The F19 key
        F20                 = 309,  // The F20 key
        F21                 = 310,  // The F21 key
        F22                 = 311,  // The F22 key
        F23                 = 312,  // The F23 key
        F24                 = 313,  // The F24 key
        F25                 = 314,  // The F25 key
        Numpad0             = 320,  // The numpad 0 key
        Numpad1             = 321,  // The numpad 1 key
        Numpad2             = 322,  // The numpad 2 key
        Numpad3             = 323,  // The numpad 3 key
        Numpad4             = 324,  // The numpad 4 key
        Numpad5             = 325,  // The numpad 5 key
        Numpad6             = 326,  // The numpad 6 key
        Numpad7             = 327,  // The numpad 7 key
        Numpad8             = 328,  // The numpad 8 key
        Numpad9             = 329,  // The numpad 9 key
        NumpadPeriod        = 330,  // The numpad . key
        NumpadDivide        = 331,  // The numpad / key
        NumpadMultiply      = 332,  // The numpad * key
        NumpadSubtract      = 333,  // The numpad - key
        NumpadAdd           = 334,  // The numpad + key
        NumpadEnter         = 335,  // The numpad Enter key
        NumpadEqual         = 336,  // The numpad = key
        LShift              = 340,  // The left Shift key
        LControl            = 341,  // The left Control key
        LAlt                = 342,  // The left Alt key
        LSuper              = 343,  // The left Super key
        RShift              = 344,  // The right Shift key
        RControl            = 345,  // The right Control key
        RAlt                = 346,  // The right Alt key
        RSuper              = 347,  // The right Super key
        Menu                = 348,  // The Menu key

        Count,
    };
    DEFINE_ENUM_TO_U64_CAST(Key);

    enum class MouseButton : i32
    {
        Button1             = 0,    // The button no. 1
        Button2             = 1,    // The button no. 2
        Button3             = 2,    // The button no. 3
        Button4             = 3,    // The button no. 4
        Button5             = 4,    // The button no. 5
        Button6             = 5,    // The button no. 6
        Button7             = 6,    // The button no. 7
        Button8             = 7,    // The button no. 8

        Count,

        Left = Button1,             // The left mouse button
        Right = Button2,            // The right mouse button
        Middle = Button3,           // The middle (wheel) mouse button
    };
    DEFINE_ENUM_TO_U64_CAST(MouseButton);

    enum class MouseWheel : i32
    {
        VerticalWheel       = 0,    // The vertical mouse wheel
        HorizontalWheel     = 1,    // The horizontal mouse wheel

        Count,
    };
    DEFINE_ENUM_TO_U64_CAST(MouseWheel);

    enum class GamepadButton : i32
    {
        ButtonA             = 0,    // The A button
        ButtonB             = 1,    // The B button
        ButtonX             = 2,    // The X button
        ButtonY             = 3,    // The Y button
        LeftBumper          = 4,    // The left bumper button
        RightBumper         = 5,    // The right bumper button
        Back                = 6,    // The back button
        Start               = 7,    // The start button
        Guide               = 8,    // The guide button
        LeftThumb           = 9,    // The left thumb button
        RightThumb          = 10,   // The right thumb button
        DPadUp              = 11,   // The d-pad up button
        DPadRight           = 12,   // The d-pad right button
        DPadDown            = 13,   // The d-pad down button
        DPadLeft            = 14,   // The d-pad left button

        Count,

        ButtonCross = ButtonA,      // The cross button
        ButtonCircle = ButtonB,     // The circle button
        ButtonSquare = ButtonX,     // The square button
        ButtonTriangle = ButtonY,   // The triangle button
    };
    DEFINE_ENUM_TO_U64_CAST(GamepadButton);

    enum class GamepadAxis : i32
    {
        LeftStickX          = 0,    // The X component of the left stick
        LeftStickY          = 1,    // The Y component of the left stick
        RightStickX         = 2,    // The X component of the right stick
        RightStickY         = 3,    // The Y component of the right stick
        LeftTrigger         = 4,    // The left trigger
        RightTrigger        = 5,    // The right trigger

        Count,
    };
    DEFINE_ENUM_TO_U64_CAST(GamepadAxis);

    enum class InputAction : u8
    {
        Released,
        Pressed,
        Repeat,
    };

    enum InputFlag : u8
    {
        InputFlag_None              = 0x00,
        InputFlag_EndedFrameDown    = 0x01,
        InputFlag_WasReleased       = 0x02,
        InputFlag_IsRepeated        = 0x04,
        InputFlag_WasHandled        = 0x08
    };

    struct InputState
    {
        u8 pressed_count = 0;
        u8 flags = InputFlag_None;
    };

    struct AnalogInputState
    {
        f32 end_value = 0.f;
        f32 max_value = 0.f;
        f32 min_value = 0.f;
    };

    struct Input
    {
        InputState keys[+Key::Count];
        InputState mouse[+MouseButton::Count];
        InputState buttons[+GamepadButton::Count];

        AnalogInputState wheels[+MouseWheel::Count];
        AnalogInputState axes[+GamepadAxis::Count];
    };

    void process_input_action(InputState* state, InputAction action);
    void process_input_value(AnalogInputState* state, f32 value);

    void poll_gamepad_inputs();
    void input_clear();

    i32 input_has_been_pressed(Key key);
    bool input_has_been_held(Key key);
    bool input_has_been_released(Key key);
    bool input_is_flag_set(Key key, InputFlag flag);

    i32 input_has_been_pressed(MouseButton button);
    bool input_has_been_held(MouseButton button);
    bool input_has_been_released(MouseButton button);
    bool input_is_flag_set(MouseButton button, InputFlag flag);

    i32 input_has_been_pressed(GamepadButton button);
    bool input_has_been_held(GamepadButton button);
    bool input_has_been_released(GamepadButton button);
    bool input_is_flag_set(GamepadButton button, InputFlag flag);

    i32 input_has_any_key_been_pressed();
    i32 input_has_any_mouse_button_been_pressed();
    i32 input_has_any_gamepad_button_been_pressed();

    AnalogInputState input_get_analog_state(MouseWheel wheel);
    AnalogInputState input_get_analog_state(GamepadAxis axis);

    Key string_to_key(String string);
}
#pragma once

#include "aporia_string.hpp"
#include "aporia_types.hpp"

enum Key : i32
{
    Key_Unknown             = -1,   // Unhandled key
    Key_Space               = 32,   // The Space key
    Key_Quote               = 39,   // The ' key
    Key_Comma               = 44,   // The , key
    Key_Minus               = 45,   // The - key
    Key_Period              = 46,   // The . key
    Key_Slash               = 47,   // The / key
    Key_Num0                = 48,   // The 0 key
    Key_Num1                = 49,   // The 1 key
    Key_Num2                = 50,   // The 2 key
    Key_Num3                = 51,   // The 3 key
    Key_Num4                = 52,   // The 4 key
    Key_Num5                = 53,   // The 5 key
    Key_Num6                = 54,   // The 6 key
    Key_Num7                = 55,   // The 7 key
    Key_Num8                = 56,   // The 8 key
    Key_Num9                = 57,   // The 9 key
    Key_Semicolon           = 59,   // The ; key
    Key_Equal               = 61,   // The = key
    Key_A                   = 65,   // The A key
    Key_B                   = 66,   // The B key
    Key_C                   = 67,   // The C key
    Key_D                   = 68,   // The D key
    Key_E                   = 69,   // The E key
    Key_F                   = 70,   // The F key
    Key_G                   = 71,   // The G key
    Key_H                   = 72,   // The H key
    Key_I                   = 73,   // The I key
    Key_J                   = 74,   // The J key
    Key_K                   = 75,   // The K key
    Key_L                   = 76,   // The L key
    Key_M                   = 77,   // The M key
    Key_N                   = 78,   // The N key
    Key_O                   = 79,   // The O key
    Key_P                   = 80,   // The P key
    Key_Q                   = 81,   // The Q key
    Key_R                   = 82,   // The R key
    Key_S                   = 83,   // The S key
    Key_T                   = 84,   // The T key
    Key_U                   = 85,   // The U key
    Key_V                   = 86,   // The V key
    Key_W                   = 87,   // The W key
    Key_X                   = 88,   // The X key
    Key_Y                   = 89,   // The Y key
    Key_Z                   = 90,   // The Z key
    Key_LBracket            = 91,   // The ( key
    Key_Backslash           = 92,   // The \ key
    Key_RBracket            = 93,   // The ) key
    Key_Tilde               = 96,   // The ~ key
    Key_World1              = 161,  // The World1 key
    Key_World2              = 162,  // The World2 key
    Key_Escape              = 256,  // The Escape key
    Key_Enter               = 257,  // The Enter/Return keys
    Key_Tab                 = 258,  // The Tab key
    Key_Backspace           = 259,  // The Backspace key
    Key_Insert              = 260,  // The Insert key
    Key_Delete              = 261,  // The Delete key
    Key_Right               = 262,  // The Right key
    Key_Left                = 263,  // The Left key
    Key_Down                = 264,  // The Down key
    Key_Up                  = 265,  // The Up key
    Key_PageUp              = 266,  // The PageUp key
    Key_PageDown            = 267,  // The PageDown key
    Key_Home                = 268,  // The Home key
    Key_End                 = 279,  // The End key
    Key_CapsLock            = 280,  // The CapsLock key
    Key_ScrollLock          = 281,  // The ScrollLock key
    Key_NumLock             = 282,  // The NumLock key
    Key_PrintScreen         = 283,  // The PrintScreen key
    Key_Pause               = 284,  // The Pause key
    Key_F1                  = 290,  // The F1 key
    Key_F2                  = 291,  // The F2 key
    Key_F3                  = 292,  // The F3 key
    Key_F4                  = 293,  // The F4 key
    Key_F5                  = 294,  // The F5 key
    Key_F6                  = 295,  // The F6 key
    Key_F7                  = 296,  // The F7 key
    Key_F8                  = 297,  // The F8 key
    Key_F9                  = 298,  // The F9 key
    Key_F10                 = 299,  // The F10 key
    Key_F11                 = 300,  // The F11 key
    Key_F12                 = 301,  // The F12 key
    Key_F13                 = 302,  // The F13 key
    Key_F14                 = 303,  // The F14 key
    Key_F15                 = 304,  // The F15 key
    Key_F16                 = 305,  // The F16 key
    Key_F17                 = 306,  // The F17 key
    Key_F18                 = 307,  // The F18 key
    Key_F19                 = 308,  // The F19 key
    Key_F20                 = 309,  // The F20 key
    Key_F21                 = 310,  // The F21 key
    Key_F22                 = 311,  // The F22 key
    Key_F23                 = 312,  // The F23 key
    Key_F24                 = 313,  // The F24 key
    Key_F25                 = 314,  // The F25 key
    Key_Numpad0             = 320,  // The numpad 0 key
    Key_Numpad1             = 321,  // The numpad 1 key
    Key_Numpad2             = 322,  // The numpad 2 key
    Key_Numpad3             = 323,  // The numpad 3 key
    Key_Numpad4             = 324,  // The numpad 4 key
    Key_Numpad5             = 325,  // The numpad 5 key
    Key_Numpad6             = 326,  // The numpad 6 key
    Key_Numpad7             = 327,  // The numpad 7 key
    Key_Numpad8             = 328,  // The numpad 8 key
    Key_Numpad9             = 329,  // The numpad 9 key
    Key_NumpadPeriod        = 330,  // The numpad . key
    Key_NumpadDivide        = 331,  // The numpad / key
    Key_NumpadMultiply      = 332,  // The numpad * key
    Key_NumpadSubtract      = 333,  // The numpad - key
    Key_NumpadAdd           = 334,  // The numpad + key
    Key_NumpadEnter         = 335,  // The numpad Enter key
    Key_NumpadEqual         = 336,  // The numpad = key
    Key_LShift              = 340,  // The left Shift key
    Key_LControl            = 341,  // The left Control key
    Key_LAlt                = 342,  // The left Alt key
    Key_LSuper              = 343,  // The left Super key
    Key_RShift              = 344,  // The right Shift key
    Key_RControl            = 345,  // The right Control key
    Key_RAlt                = 346,  // The right Alt key
    Key_RSuper              = 347,  // The right Super key
    Key_Menu                = 348,  // The Menu key

    Key_Count,
};

enum MouseButton : i32
{
    Mouse_Button1           = 0,    // The button no. 1
    Mouse_Button2           = 1,    // The button no. 2
    Mouse_Button3           = 2,    // The button no. 3
    Mouse_Button4           = 3,    // The button no. 4
    Mouse_Button5           = 4,    // The button no. 5
    Mouse_Button6           = 5,    // The button no. 6
    Mouse_Button7           = 6,    // The button no. 7
    Mouse_Button8           = 7,    // The button no. 8

    Mouse_Count,

    Mouse_Left = Mouse_Button1,     // The left mouse button
    Mouse_Right = Mouse_Button2,    // The right mouse button
    Mouse_Middle = Mouse_Button3,   // The middle (wheel) mouse button
};

enum MouseWheel : i32
{
    Mouse_VerticalWheel     = 0,    // The vertical mouse wheel
    Mouse_HorizontalWheel   = 1,    // The horizontal mouse wheel

    MouseWheel_Count,
};

enum GamepadButton : i32
{
    Gamepad_ButtonA         = 0,    // The A button
    Gamepad_ButtonB         = 1,    // The B button
    Gamepad_ButtonX         = 2,    // The X button
    Gamepad_ButtonY         = 3,    // The Y button
    Gamepad_LeftBumper      = 4,    // The left bumper button
    Gamepad_RightBumper     = 5,    // The right bumper button
    Gamepad_Back            = 6,    // The back button
    Gamepad_Start           = 7,    // The start button
    Gamepad_Guide           = 8,    // The guide button
    Gamepad_LeftThumb       = 9,    // The left thumb button
    Gamepad_RightThumb      = 10,   // The right thumb button
    Gamepad_DPadUp          = 11,   // The d-pad up button
    Gamepad_DPadRight       = 12,   // The d-pad right button
    Gamepad_DPadDown        = 13,   // The d-pad down button
    Gamepad_DPadLeft        = 14,   // The d-pad left button

    Gamepad_Count,

    Gamepad_Cross = Gamepad_ButtonA,    // The cross button
    Gamepad_Circle = Gamepad_ButtonB,   // The circle button
    Gamepad_Square = Gamepad_ButtonX,   // The square button
    Gamepad_Triangle = Gamepad_ButtonY, // The triangle button
};

enum GamepadAnalog : i32
{
    Gamepad_LeftStickX      = 0,    // The X component of the left stick
    Gamepad_LeftStickY      = 1,    // The Y component of the left stick
    Gamepad_RightStickX     = 2,    // The X component of the right stick
    Gamepad_RightStickY     = 3,    // The Y component of the right stick
    Gamepad_LeftTrigger     = 4,    // The left trigger
    Gamepad_RightTrigger    = 5,    // The right trigger

    GamepadAnalog_Count,
};

enum InputAction : u8
{
    InputAction_Released,
    InputAction_Pressed,
    InputAction_Repeat,
};

using InputFlags = u8;
enum InputFlag_ : InputFlags
{
    InputFlag_None              = 0x00,
    InputFlag_EndedFrameDown    = 0x01,
    InputFlag_WasReleased       = 0x02,
    InputFlag_IsRepeated        = 0x04,
};

enum InputOwner : u8
{
    InputOwner_None             = 0x00,

    InputOwner_Game             = 0x01,
    InputOwner_UI               = 0x02,

#if defined(APORIA_EDITOR)
    InputOwner_Editor           = 0xfd,
    InputOwner_CommandLine      = 0xfe,
#endif

#if defined(APORIA_IMGUI)
    InputOwner_ImGui            = 0xff,
#endif
};

struct InputState
{
    u8 pressed_count = 0;
    InputFlags flags = InputFlag_None;

    InputOwner owner = InputOwner_None;
    InputOwner last_owner = InputOwner_None;
};

struct AnalogInputState
{
    f32 end_value = 0.f;
    f32 max_value = 0.f;
    f32 min_value = 0.f;
};

struct Input
{
    InputState keys[Key_Count];
    InputState mouse[Mouse_Count];
    InputState gamepad[Gamepad_Count];

    AnalogInputState wheels[MouseWheel_Count];
    AnalogInputState analogs[GamepadAnalog_Count];
};

void input_process_key_event(Key key, InputAction action);
void input_process_mouse_event(MouseButton button, InputAction action);
void input_process_scroll_event(MouseWheel wheel, f32 value);

void input_clear();
void input_process_events();

extern InputOwner active_owner;

#define input_push_active_owner(owner) \
    InputOwner CONCAT(_last_owner, __LINE__) = active_owner; \
    active_owner = owner; \
    defer { active_owner = CONCAT(_last_owner, __LINE__); };

InputState input_get(Key key);
InputState input_get(MouseButton button);
InputState input_get(GamepadButton button);

AnalogInputState input_get(MouseWheel wheel);
AnalogInputState input_get(GamepadAnalog analog);

bool input_is_pressed(InputState state);
bool input_is_held(InputState state);
bool input_is_released(InputState state);
bool input_is_repeated(InputState state);

bool input_is_pressed(Key key);
bool input_is_held(Key key);
bool input_is_released(Key key);
bool input_is_repeated(Key key);

bool input_is_pressed(MouseButton button);
bool input_is_held(MouseButton button);
bool input_is_released(MouseButton button);
bool input_is_repeated(MouseButton button);

bool input_is_pressed(GamepadButton button);
bool input_is_held(GamepadButton button);
bool input_is_released(GamepadButton button);
bool input_is_repeated(GamepadButton button);

bool input_is_any_key_pressed();
bool input_is_any_mouse_button_pressed();
bool input_is_any_gamepad_button_pressed();

Key string_to_key(String string);

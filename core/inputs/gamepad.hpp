#pragma once

#include <string_view>

#include "aporia_types.hpp"

namespace Aporia
{
    enum class Gamepad : i32
    {
        ButtonA = 0,                /* The A button */
        ButtonB = 1,                /* The B button */
        ButtonX = 2,                /* The X button */
        ButtonY = 3,                /* The Y button */
        LeftBumper = 4,             /* The left bumper button */
        RightBumper = 5,            /* The right bumper button */
        Back = 6,                   /* The back button */
        Start = 7,                  /* The start button */
        Guide = 8,                  /* The guide button */
        LeftThumb = 9,              /* The left thumb button */
        RightThumb = 10,            /* The right thumb button */
        DPadUp = 11,                /* The d-pad up button */
        DPadRight = 12,             /* The d-pad right button */
        DPadDown = 13,              /* The d-pad down button */
        DPadLeft = 14,              /* The d-pad left button */

        ButtonCross = ButtonA,      /* The cross button */
        ButtonCircle = ButtonB,     /* The circle button */
        ButtonSquare = ButtonX,     /* The square button */
        ButtonTriangle = ButtonY    /* The triangle button */
    };

    enum class GamepadAxis : i32
    {
        LeftStickX = 0,             /* The X component of the left stick */
        LeftStickY = 1,             /* The Y component of the left stick */
        RightStickX = 2,            /* The X component of the right stick */
        RightStickY = 3,            /* The Y component of the right stick */
        LeftTrigger = 4,            /* The left trigger */
        RightTrigger = 5            /* The right trigger */
    };

    constexpr std::string_view gamepadcode_name(Gamepad button)
    {
        return magic_enum::enum_name(button);
    }

    constexpr std::string_view axiscode_name(GamepadAxis button)
    {
        return magic_enum::enum_name(button);
    }
}

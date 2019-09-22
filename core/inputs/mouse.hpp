#pragma once

namespace Aporia
{
    enum class Mouse : size_t
    {
        Left,       /* The left mouse button */
        Right,      /* The right mouse button */
        Middle,     /* The middle (wheel) mouse button */
        XButton1,   /* The first extra mouse button */
        XButton2    /* The second extra mouse button */
    };

    enum class MouseWheel : size_t
    {
        VerticalWheel,  /* The vertical mouse wheel */
        HorizontalWheel /* The horizontal mouse wheel */
    };

    constexpr const char* buttoncode_name(Mouse button)
    {
        switch (button)
        {
            case Mouse::Left:       return "Left";
            case Mouse::Right:      return "Right";
            case Mouse::Middle:     return "Middle";
            case Mouse::XButton1:   return "XButton1";
            case Mouse::XButton2:   return "XButton2";
            default:                return "None";
        }
    }

    constexpr const char* wheelcode_name(MouseWheel wheel)
    {
        switch (wheel)
        {
            case MouseWheel::VerticalWheel:      return "VerticalWheel";
            case MouseWheel::HorizontalWheel:    return "HorizontalWheel";
            default:                             return "None";
        }
    }
}

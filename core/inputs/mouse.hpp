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
}

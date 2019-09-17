#pragma once

namespace Aporia
{
    enum class Mouse : int
    {
        Left,       /* The left mouse button */
        Right,      /* The right mouse button */
        Middle,     /* The middle (wheel) mouse button */
        XButton1,   /* The first extra mouse button */
        XButton2,   /* The second extra mouse button */

        ButtonCount /* Keep last -- the total number of mouse buttons */
    };

    enum class MouseWheel : int
    {
        VerticalWheel,  /* The vertical mouse wheel */
        HorizontalWheel /* The horizontal mouse wheel */
    };
}

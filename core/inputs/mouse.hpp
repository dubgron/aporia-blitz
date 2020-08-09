#pragma once

#include <cstdint>
#include <string_view>

namespace Aporia
{
    enum class Mouse : int32_t
    {
        Button1 = 0,        /* The button no. 1 */
        Button2 = 1,        /* The button no. 2 */
        Button3 = 2,        /* The button no. 3 */
        Button4 = 3,        /* The button no. 4 */
        Button5 = 4,        /* The button no. 5 */
        Button6 = 5,        /* The button no. 6 */
        Button7 = 6,        /* The button no. 7 */
        Button8 = 7,        /* The button no. 8 */
        Left = Button1,     /* The left mouse button */
        Right = Button2,    /* The right mouse button */
        Middle = Button3    /* The middle (wheel) mouse button */
    };

    enum class MouseWheel : int32_t
    {
        VerticalWheel = 0,     /* The vertical mouse wheel */
        HorizontalWheel = 1    /* The horizontal mouse wheel */
    };

    constexpr std::string_view buttoncode_name(Mouse button)
    {
        return magic_enum::enum_name(button);
    }

    constexpr std::string_view wheelcode_name(MouseWheel wheel = MouseWheel::VerticalWheel)
    {
        return magic_enum::enum_name(wheel);
    }
}

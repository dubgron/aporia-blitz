#pragma once

namespace Aporia
{
    enum class Keyboard : size_t
    {
        A,            /* The A key */
        B,            /* The B key */
        C,            /* The C key */
        D,            /* The D key */
        E,            /* The E key */
        F,            /* The F key */
        G,            /* The G key */
        H,            /* The H key */
        I,            /* The I key */
        J,            /* The J key */
        K,            /* The K key */
        L,            /* The L key */
        M,            /* The M key */
        N,            /* The N key */
        O,            /* The O key */
        P,            /* The P key */
        Q,            /* The Q key */
        R,            /* The R key */
        S,            /* The S key */
        T,            /* The T key */
        U,            /* The U key */
        V,            /* The V key */
        W,            /* The W key */
        X,            /* The X key */
        Y,            /* The Y key */
        Z,            /* The Z key */
        Num0,         /* The 0 key */
        Num1,         /* The 1 key */
        Num2,         /* The 2 key */
        Num3,         /* The 3 key */
        Num4,         /* The 4 key */
        Num5,         /* The 5 key */
        Num6,         /* The 6 key */
        Num7,         /* The 7 key */
        Num8,         /* The 8 key */
        Num9,         /* The 9 key */
        Escape,       /* The Escape key */
        LControl,     /* The left Control key */
        LShift,       /* The left Shift key */
        LAlt,         /* The left Alt key */
        LSystem,      /* The left OS specific key: window (Windows and Linux), apple (MacOS X), ... */
        RControl,     /* The right Control key */
        RShift,       /* The right Shift key */
        RAlt,         /* The right Alt key */
        RSystem,      /* The right OS specific key: window (Windows and Linux), apple (MacOS X), ... */
        Menu,         /* The Menu key */
        LBracket,     /* The [ key */
        RBracket,     /* The ] key */
        Semicolon,    /* The ; key */
        Comma,        /* The , key */
        Period,       /* The . key */
        Quote,        /* The ' key */
        Slash,        /* The / key */
        Backslash,    /* The \ key */
        Tilde,        /* The ~ key */
        Equal,        /* The = key */
        Hyphen,       /* The - key (hyphen) */
        Space,        /* The Space key */
        Enter,        /* The Enter/Return keys */
        Backspace,    /* The Backspace key */
        Tab,          /* The Tabulation key */
        PageUp,       /* The Page up key */
        PageDown,     /* The Page down key */
        End,          /* The End key */
        Home,         /* The Home key */
        Insert,       /* The Insert key */
        Delete,       /* The Delete key */
        Add,          /* The + key */
        Subtract,     /* The - key (minus, usually from numpad) */
        Multiply,     /* The * key */
        Divide,       /* The / key */
        Left,         /* Left arrow */
        Right,        /* Right arrow */
        Up,           /* Up arrow */
        Down,         /* Down arrow */
        Numpad0,      /* The numpad 0 key */
        Numpad1,      /* The numpad 1 key */
        Numpad2,      /* The numpad 2 key */
        Numpad3,      /* The numpad 3 key */
        Numpad4,      /* The numpad 4 key */
        Numpad5,      /* The numpad 5 key */
        Numpad6,      /* The numpad 6 key */
        Numpad7,      /* The numpad 7 key */
        Numpad8,      /* The numpad 8 key */
        Numpad9,      /* The numpad 9 key */
        F1,           /* The F1 key */
        F2,           /* The F2 key */
        F3,           /* The F3 key */
        F4,           /* The F4 key */
        F5,           /* The F5 key */
        F6,           /* The F6 key */
        F7,           /* The F7 key */
        F8,           /* The F8 key */
        F9,           /* The F9 key */
        F10,          /* The F10 key */
        F11,          /* The F11 key */
        F12,          /* The F12 key */
        F13,          /* The F13 key */
        F14,          /* The F14 key */
        F15,          /* The F15 key */
        Pause         /* The Pause key */
    };

    constexpr const char* keycode_name(Keyboard key)
    {
        switch (key)
        {
            case Keyboard::A:           return "A";
            case Keyboard::B:           return "B";
            case Keyboard::C:           return "C";
            case Keyboard::D:           return "D";
            case Keyboard::E:           return "E";
            case Keyboard::F:           return "F";
            case Keyboard::G:           return "G";
            case Keyboard::H:           return "H";
            case Keyboard::I:           return "I";
            case Keyboard::J:           return "J";
            case Keyboard::K:           return "K";
            case Keyboard::L:           return "L";
            case Keyboard::M:           return "M";
            case Keyboard::N:           return "N";
            case Keyboard::O:           return "O";
            case Keyboard::P:           return "P";
            case Keyboard::Q:           return "Q";
            case Keyboard::R:           return "R";
            case Keyboard::S:           return "S";
            case Keyboard::T:           return "T";
            case Keyboard::U:           return "U";
            case Keyboard::V:           return "V";
            case Keyboard::W:           return "W";
            case Keyboard::X:           return "X";
            case Keyboard::Y:           return "Y";
            case Keyboard::Z:           return "Z";
            case Keyboard::Num0:        return "Num0";
            case Keyboard::Num1:        return "Num1";
            case Keyboard::Num2:        return "Num2";
            case Keyboard::Num3:        return "Num3";
            case Keyboard::Num4:        return "Num4";
            case Keyboard::Num5:        return "Num5";
            case Keyboard::Num6:        return "Num6";
            case Keyboard::Num7:        return "Num7";
            case Keyboard::Num8:        return "Num8";
            case Keyboard::Num9:        return "Num9";
            case Keyboard::Escape:      return "Escape";
            case Keyboard::LControl:    return "LControl";
            case Keyboard::LShift:      return "LShift";
            case Keyboard::LAlt:        return "LAlt";
            case Keyboard::LSystem:     return "LSystem";
            case Keyboard::RControl:    return "RControl";
            case Keyboard::RShift:      return "RShift";
            case Keyboard::RAlt:        return "RAlt";
            case Keyboard::RSystem:     return "RSystem";
            case Keyboard::Menu:        return "Menu";
            case Keyboard::LBracket:    return "LBracket";
            case Keyboard::RBracket:    return "RBracket";
            case Keyboard::Semicolon:   return "Semicolon";
            case Keyboard::Comma:       return "Comma";
            case Keyboard::Period:      return "Period";
            case Keyboard::Quote:       return "Quote";
            case Keyboard::Slash:       return "Slash";
            case Keyboard::Backslash:   return "Backslash";
            case Keyboard::Tilde:       return "Tilde";
            case Keyboard::Equal:       return "Equal";
            case Keyboard::Hyphen:      return "Hyphen";
            case Keyboard::Space:       return "Space";
            case Keyboard::Enter:       return "Enter";
            case Keyboard::Backspace:   return "Backspace";
            case Keyboard::Tab:         return "Tab";
            case Keyboard::PageUp:      return "PageUp";
            case Keyboard::PageDown:    return "PageDown";
            case Keyboard::End:         return "End";
            case Keyboard::Home:        return "Home";
            case Keyboard::Insert:      return "Insert";
            case Keyboard::Delete:      return "Delete";
            case Keyboard::Add:         return "Add";
            case Keyboard::Subtract:    return "Subtract";
            case Keyboard::Multiply:    return "Multiply";
            case Keyboard::Divide:      return "Divide";
            case Keyboard::Left:        return "Left";
            case Keyboard::Right:       return "Right";
            case Keyboard::Up:          return "Up";
            case Keyboard::Down:        return "Down";
            case Keyboard::Numpad0:     return "Numpad0";
            case Keyboard::Numpad1:     return "Numpad1";
            case Keyboard::Numpad2:     return "Numpad2";
            case Keyboard::Numpad3:     return "Numpad3";
            case Keyboard::Numpad4:     return "Numpad4";
            case Keyboard::Numpad5:     return "Numpad5";
            case Keyboard::Numpad6:     return "Numpad6";
            case Keyboard::Numpad7:     return "Numpad7";
            case Keyboard::Numpad8:     return "Numpad8";
            case Keyboard::Numpad9:     return "Numpad9";
            case Keyboard::F1:          return "F1";
            case Keyboard::F2:          return "F2";
            case Keyboard::F3:          return "F3";
            case Keyboard::F4:          return "F4";
            case Keyboard::F5:          return "F5";
            case Keyboard::F6:          return "F6";
            case Keyboard::F7:          return "F7";
            case Keyboard::F8:          return "F8";
            case Keyboard::F9:          return "F9";
            case Keyboard::F10:         return "F10";
            case Keyboard::F11:         return "F11";
            case Keyboard::F12:         return "F12";
            case Keyboard::F13:         return "F13";
            case Keyboard::F14:         return "F14";
            case Keyboard::F15:         return "F15";
            case Keyboard::Pause:       return "Pause";
            default:                    return "None";
        }
    }
}
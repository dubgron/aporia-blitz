#include "aporia_utils.hpp"

#include "aporia_debug.hpp"
#include "aporia_game.hpp"

namespace Aporia
{
    String read_file(MemoryArena* arena, String filepath)
    {
        FILE* file = fopen(*filepath, "rb");

        if (file == nullptr)
        {
            APORIA_LOG(Error, "Failed to open file '%'!", filepath);
            return String{};
        }

        APORIA_LOG(Info, "Opened file '%' successfully!", filepath);
        fseek(file, 0, SEEK_END);

        u64 length = ftell(file);
        u8* data = arena->push<u8>(length + 1);

        fseek(file, 0, SEEK_SET);
        fread(data, length, 1, file);
        data[length] = '\0';

        fclose(file);

        return String{ data, length };
    }

    String replace_extension(MemoryArena* arena, String filepath, String ext)
    {
        u64 last_fullstop = filepath.rfind('.');
        filepath = filepath.substr(0, last_fullstop);

        String result = push_string(arena, filepath.length + ext.length + 1);
        memcpy(result.data, filepath.data, filepath.length);
        result.data[filepath.length] = '.';
        memcpy(result.data + filepath.length + 1, ext.data, ext.length);
        return result;
    }

    String extract_filename(String filepath)
    {
        u64 last_slash = filepath.rfind('/');
        if (last_slash == String::INVALID_INDEX)
        {
            last_slash = filepath.rfind('\\');
        }

        if (last_slash == String::INVALID_INDEX)
        {
            return filepath;
        }
        else
        {
            return filepath.substr(last_slash + 1);
        }
    }

    const Color Color::Black       = Color{  0,   0,   0,  255 };
    const Color Color::White       = Color{ 255, 255, 255, 255 };
    const Color Color::Red         = Color{ 255,  0,   0,  255 };
    const Color Color::Green       = Color{  0,  255,  0,  255 };
    const Color Color::Blue        = Color{  0,   0,  255, 255 };
    const Color Color::Yellow      = Color{ 255, 255,  0,  255 };
    const Color Color::Magenta     = Color{ 255,  0,  255, 255 };
    const Color Color::Cyan        = Color{  0,  255, 255, 255 };
    const Color Color::Transparent = Color{  0,   0,   0,   0  };

    Color color_from_vec4(f64 r, f64 g, f64 b, f64 a)
    {
        const u8 new_r = static_cast<u8>(r * 255);
        const u8 new_g = static_cast<u8>(g * 255);
        const u8 new_b = static_cast<u8>(b * 255);
        const u8 new_a = static_cast<u8>(a * 255);

        return Color{ new_r, new_g, new_b, new_a };
    }

    Color hsv_to_rgb(i64 hue, f64 saturation, f64 value)
    {
        hue = unwind_angle(hue);
        APORIA_ASSERT(hue >= 0 && hue < 360);

        const f64 C = saturation * value;
        const f64 X = C * (1.0 - std::abs(std::fmod(hue / 60.0, 2.0) - 1.0));
        const f64 m = value - C;

        f64 Rs, Gs, Bs;
        if (hue >= 0 && hue < 60)
        {
            Rs = C;
            Gs = X;
            Bs = 0.0;
        }
        else if (hue >= 60 && hue < 120)
        {
            Rs = X;
            Gs = C;
            Bs = 0.0;
        }
        else if (hue >= 120 && hue < 180)
        {
            Rs = 0.0;
            Gs = C;
            Bs = X;
        }
        else if (hue >= 180 && hue < 240)
        {
            Rs = 0.0;
            Gs = X;
            Bs = C;
        }
        else if (hue >= 240 && hue < 300)
        {
            Rs = X;
            Gs = 0.0;
            Bs = C;
        }
        else if (hue >= 300 && hue < 360)
        {
            Rs = C;
            Gs = 0.0;
            Bs = X;
        }
        else
        {
            APORIA_UNREACHABLE();
        }

        const u8 r = static_cast<u8>((Rs + m) * 255);
        const u8 g = static_cast<u8>((Gs + m) * 255);
        const u8 b = static_cast<u8>((Bs + m) * 255);

        return Color{ r, g, b };
    }

    f32 degrees_to_radians(f32 angle_in_degrees)
    {
        return angle_in_degrees / 180.f * M_PI;
    }

    f32 radians_to_degrees(f32 angle_in_radians)
    {
        return angle_in_radians / M_PI * 180.f;
    }

    i32 unwind_angle(i64 angle)
    {
        constexpr i32 full_angle = 360;
        return (full_angle + (angle % full_angle)) % full_angle;
    }

    static std::mt19937 random_generator{ std::random_device{}() };

    i32 random_range(i32 min, i32 max)
    {
        return std::uniform_int_distribution(min, max)(random_generator);
    }

    i64 random_range(i64 min, i64 max)
    {
        return std::uniform_int_distribution(min, max)(random_generator);
    }

    f32 random_range(f32 min, f32 max)
    {
        return std::uniform_real_distribution(min, max)(random_generator);
    }
}

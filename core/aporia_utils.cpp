#include "aporia_utils.hpp"

#include "aporia_debug.hpp"
#include "aporia_game.hpp"

String read_entire_file(MemoryArena* arena, String filepath)
{
    FILE* file = fopen(*filepath, "rb");

    if (file == nullptr)
    {
        APORIA_LOG(Error, "Failed to open file '%'!", filepath);
        return String{};
    }

    APORIA_LOG(Info, "Opened file '%' successfully!", filepath);

    fseek(file, 0, SEEK_END);
    u64 size_in_bytes = ftell(file);

    u8* data = arena_push_uninitialized<u8>(arena, size_in_bytes);

    fseek(file, 0, SEEK_SET);
    fread(data, size_in_bytes, 1, file);

    fclose(file);

    return String{ data, size_in_bytes };
}

String read_entire_text_file(MemoryArena* arena, String filepath)
{
    String result = read_entire_file(arena, filepath);

    u64 file_size = result.length;
    fix_eol(&result);

    u64 garbage_bytes = file_size - result.length;
    if (garbage_bytes > 0)
    {
        arena_pop(arena, garbage_bytes);
    }

    return result;
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
    if (last_slash == INDEX_INVALID)
    {
        last_slash = filepath.rfind('\\');
    }

    if (last_slash == INDEX_INVALID)
    {
        return filepath;
    }
    else
    {
        return filepath.substr(last_slash + 1);
    }
}

void fix_path_slashes(String* filepath)
{
    for (u64 idx = 0; idx < filepath->length; ++idx)
    {
        if (filepath->data[idx] == '\\')
        {
            filepath->data[idx] = '/';
        }
    }
}

void fix_eol(String* filepath)
{
    i64 dst = 0, src = 0;
    while (src < filepath->length)
    {
        if (filepath->data[src] == '\r')
        {
            src += 1;
            continue;
        }

        filepath->data[dst++] = filepath->data[src++];
    }
    filepath->length = dst;
}

constexpr u64 FNV_64_PRIME = 0x100000001b3;
constexpr u64 FNV_64_OFFSET_BIAS = 0xcbf29ce484222325;

static u64 fnv1a_hash(u64 value, u64 hash = FNV_64_OFFSET_BIAS)
{
    hash ^= value;
    return hash * FNV_64_PRIME;
}

static u64 fnv1a_hash(String string, u64 hash = FNV_64_OFFSET_BIAS)
{
    for (u64 idx = 0; idx < string.length; ++idx)
    {
        hash = fnv1a_hash(string.data[idx], hash);
    }
    return hash;
}

u32 get_hash(String string)
{
    return (u32)fnv1a_hash(string);
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

Color color_from_vec4(f32 r, f32 g, f32 b, f32 a)
{
    u8 new_r = static_cast<u8>(r * 255);
    u8 new_g = static_cast<u8>(g * 255);
    u8 new_b = static_cast<u8>(b * 255);
    u8 new_a = static_cast<u8>(a * 255);

    return Color{ new_r, new_g, new_b, new_a };
}

Color hsv_to_rgb(i32 hue, f32 saturation, f32 value)
{
    constexpr i32 full_circle = 360;
    hue = wrap_around(hue, full_circle);

    f32 C = saturation * value;
    f32 X = C * (1.0 - abs(fmod(hue / 60.0, 2.0) - 1.0));
    f32 m = value - C;

    f32 Rs, Gs, Bs;
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

    u8 r = static_cast<u8>((Rs + m) * 255);
    u8 g = static_cast<u8>((Gs + m) * 255);
    u8 b = static_cast<u8>((Bs + m) * 255);

    return Color{ r, g, b };
}

void rgb_to_hsv(Color rgb, i32* hue, f32* saturation, f32* value)
{
    f32 Rs = rgb.r / 255.f;
    f32 Gs = rgb.g / 255.f;
    f32 Bs = rgb.b / 255.f;

    f32 C_max = max(max(Rs, Gs), Bs);
    f32 C_min = min(min(Rs, Gs), Bs);
    f32 delta = C_max - C_min;

    if (delta == 0)
    {
        *hue = 0;
    }
    else if (C_max == Rs)
    {
        *hue = 60 * fmod((Gs - Bs) / delta, 6.f);
    }
    else if (C_max == Gs)
    {
        *hue = 60 * ((Bs - Rs) / delta + 2);
    }
    else if (C_max == Bs)
    {
        *hue = 60 * ((Rs - Gs) / delta + 4);
    }

    if (C_max == 0)
    {
        *saturation = 0;
    }
    else
    {
        *saturation = delta / C_max;
    }

    *value = C_max;
}

Color lerp(Color c0, Color c1, f32 t)
{
    Color result;
    result.r = lerp(c0.r, c1.r, t);
    result.g = lerp(c0.g, c1.g, t);
    result.b = lerp(c0.b, c1.b, t);
    result.a = lerp(c0.a, c1.a, t);
    return result;
}

f32 degrees_to_radians(f32 angle_in_degrees)
{
    return angle_in_degrees / 180.f * M_PI;
}

f32 radians_to_degrees(f32 angle_in_radians)
{
    return angle_in_radians / M_PI * 180.f;
}

static std::mt19937 random_generator{ std::random_device{}() };

i32 random_range(i32 min, i32 max)
{
    if (min == max) return min;
    APORIA_ASSERT(min < max);
    return std::uniform_int_distribution(min, max)(random_generator);
}

i64 random_range(i64 min, i64 max)
{
    if (min == max) return min;
    APORIA_ASSERT(min < max);
    return std::uniform_int_distribution(min, max)(random_generator);
}

f32 random_range(f32 min, f32 max)
{
    if (min == max) return min;
    APORIA_ASSERT(min < max);
    return std::uniform_real_distribution(min, max)(random_generator);
}

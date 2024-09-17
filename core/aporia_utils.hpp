#pragma once

#include <chrono>
#include <random>

#include "aporia_debug.hpp"
#include "aporia_memory.hpp"
#include "aporia_string.hpp"
#include "aporia_types.hpp"

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(*arr))
#define ARRAY_ZERO(arr) memset(arr, 0, sizeof(arr))
#define INDEX_IN_ARRAY(elem, arr) ((PTR_TO_INT(elem) - PTR_TO_INT(arr)) / sizeof(*elem))

#define CONCAT_HELPER(x, y) x##y
#define CONCAT(x, y) CONCAT_HELPER(x, y)

template<typename F>
struct _DeferStruct
{
    _DeferStruct(F code) : code(code) {}
    ~_DeferStruct() { code(); }
    F code;
};

#define defer _DeferStruct CONCAT(_defer, __LINE__) = [&]

String read_entire_file(MemoryArena* arena, String filepath);
String read_entire_text_file(MemoryArena* arena, String filepath);

template<typename T, typename... Ts>
[[nodiscard]] String sprintf(MemoryArena* arena, String format, T arg, Ts... args)
{
    ScratchArena temp = scratch_begin(arena);
    defer { scratch_end(temp); };

    StringList builder;

    u64 args_begin = format.find('%');
    while (args_begin < format.length - 1 && format.data[args_begin + 1] == '%')
    {
        args_begin = format.find('%', args_begin + 2);
    }

    String before_args = format.substr(0, args_begin);
    if (before_args.length > 0)
    {
        builder.push_node(temp.arena, before_args);
    }

    if (args_begin < format.length)
    {
        String arg_as_string = to_string(temp.arena, arg);
        builder.push_node(temp.arena, arg_as_string);

        String after_args = format.substr(args_begin + 1);
        if (after_args.length > 0)
        {
            if constexpr (sizeof...(args) > 0)
            {
                after_args = sprintf(temp.arena, after_args, std::forward<Ts>(args)...);
            }

            builder.push_node(temp.arena, after_args);
        }
    }

    return builder.join(arena);
}

template<typename T, typename... Ts>
[[nodiscard]] String tprintf(String format, T arg, Ts... args)
{
    return sprintf(&memory.frame, format, arg, args...);
}

String replace_extension(MemoryArena* arena, String filepath, String ext);
String extract_filename(String filepath);

// @NOTE(dubgron): Careful! It's an in-place transformation.
void fix_path_slashes(String* filepath);
void fix_eol(String* filepath);

u32 get_hash(String string);
u32 get_hash(void* data, u64 size);

struct Color
{
    u8 r = 255;
    u8 g = 255;
    u8 b = 255;
    u8 a = 255;

    static const Color Black;
    static const Color White;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Magenta;
    static const Color Cyan;
    static const Color Transparent;
};

bool operator==(Color c0, Color c1)
{
    return c0.r == c1.r && c0.g == c1.g && c0.b == c1.b && c0.a == c1.a;
}

constexpr i32 HUE_RED       = 0;
constexpr i32 HUE_YELLOW    = 60;
constexpr i32 HUE_GREEN     = 120;
constexpr i32 HUE_CYAN      = 180;
constexpr i32 HUE_BLUE      = 240;
constexpr i32 HUE_MAGENTA   = 300;

Color color_from_vec4(f32 r, f32 g, f32 b, f32 a);
Color color_from_vec4(v4 vec);
v4 vec4_from_color(Color color);

Color hsv_to_rgb(i32 hue, f32 saturation, f32 value);
void rgb_to_hsv(Color rgb, i32* hue, f32* saturation, f32* value);

Color lerp(Color c0, Color c1, f32 t);

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using Duration = Clock::duration;

using Hours = std::chrono::duration<f32, std::ratio<3600>>;
using Minutes = std::chrono::duration<f32, std::ratio<60>>;
using Seconds = std::chrono::duration<f32>;
using Milliseconds = std::chrono::milliseconds;
using Microseconds = std::chrono::microseconds;
using Nanoseconds = std::chrono::nanoseconds;

struct Timer
{
    TimePoint start_time = Clock::now();

    template<typename T = Seconds>
    f32 reset()
    {
        f32 elapsed = get_elapsed_time<T>();
        start_time = Clock::now();
        return elapsed;
    }

    template<typename T = Seconds>
    f32 get_elapsed_time() const
    {
        return std::chrono::duration_cast<T>(Clock::now() - start_time).count();
    }
};

template<typename T> requires std::is_scalar_v<T>
T min(T a, T b)
{
    return (a > b) ? b : a;
}

template<typename T> requires std::is_scalar_v<T>
T max(T a, T b)
{
    return (a < b) ? b : a;
}

template<typename T> requires std::is_scalar_v<T>
T clamp(T value, T min_bound, T max_bound)
{
    return min(max(value, min_bound), max_bound);
}

f32 degrees_to_radians(f32 angle_in_degrees);
f32 radians_to_degrees(f32 angle_in_radians);

i32 random_range(i32 min, i32 max);
i64 random_range(i64 min, i64 max);
f32 random_range(f32 min, f32 max);

template<typename E, typename T = std::underlying_type_t<E>> requires std::is_enum_v<E>
constexpr T to_underlying(E enum_value)
{
    return static_cast<T>(enum_value);
}

template<typename T>
T lerp(T a, T b, f32 t)
{
    return a + (b - a) * t;
}

template<typename T>
T inverse_lerp(T a, T b, f32 t)
{
    return (t - a) / (b - a);
}

template<typename T>
T smoothstep(T a, T b, f32 t)
{
    t = inverse_lerp(a, b, t);
    t = clamp(t, 0.f, 1.f);
    t = (3.f  - 2.f * t) * t * t;
    return t;
}

template<typename T> requires std::is_integral_v<T>
T wrap_around(T index, T count)
{
    APORIA_ASSERT(count != 0);
    T quotient = div(index, count).quot;
    T remainder = index - (quotient * count);
    return remainder;
}

template<typename T> requires std::is_floating_point_v<T>
T wrap_around(T index, T count)
{
    APORIA_ASSERT(count != 0);
    T quotient = floor(index / count);
    T remainder = index - (quotient * count);
    return remainder;
}

template<typename T>
void ring_buffer_increment_index(T* index, T count)
{
    *index += 1;
    if (*index == count) *index = 0;
}

template<typename T>
void ring_buffer_decrement_index(T* index, T count)
{
    *index -= 1;
    if (*index == -1) *index = count - 1;
}

template<typename F, typename T>
concept CompFn = std::is_invocable_r_v<i32, F, const T*, const T*>;

template<typename T, CompFn<T> F>
void insertion_sort(T* data, i64 count, F comp)
{
    for (i64 i = 1; i < count; ++i)
    {
        T tmp = data[i];
        i64 j = i;
        while (j > 0 && comp(&data[j - 1], &tmp) > 0)
        {
            data[j] = data[j - 1];
            j -= 1;
        }
        data[j] = tmp;
    }
}

template<typename T>
inline void swap(T* a, T* b)
{
    T tmp = *a;
    *a = *b;
    *b = tmp;
}

template<typename T, CompFn<T> F>
void heap_sort(T* data, i64 count, F comp)
{
    i64 start = floor(count / 2);
    i64 end = count;
    while (end > 1)
    {
        if (start > 0)
        {
            start -= 1;
        }
        else
        {
            end -= 1;
            swap(&data[0], &data[end]);
        }

        i64 root = start;
        i64 child = root * 2 + 1;
        while (child < end)
        {
            if (child + 1 < end && comp(&data[child], &data[child + 1]) < 0)
            {
                child += 1;
            }

            if (comp(&data[root], &data[child]) < 0)
            {
                swap(&data[root], &data[child]);
                root = child;
            }
            else
            {
                break;
            }

            child = root * 2 + 1;
        }
    }
}

template<typename T, CompFn<T> F>
i64 partition(T* data, i64 count, F comp)
{
    i64 pivot = floor(count / 2);

    i64 i = 0;
    i64 j = count - 1;

    while (true)
    {
        while (comp(&data[i], &data[pivot]) < 0) i += 1;
        while (comp(&data[pivot], &data[j]) < 0) j -= 1;

        if (i >= j)
            return i;

        swap(&data[i], &data[j]);

        if (pivot == i) pivot = j;
        else if (pivot == j) pivot = i;

        i += 1;
        j -= 1;
    }

    return -1;
}

template<typename T, CompFn<T> F>
void quick_sort(T* data, i64 count, F comp)
{
    if (count < 2)
        return;

    i64 p = partition(data, count, comp);

    quick_sort(data, p, comp);
    quick_sort(data + p, count - p, comp);
}

template<typename T, CompFn<T> F>
void intro_sort(T* data, i64 count, F comp, i64 max_depth)
{
    if (count < 16)
    {
        insertion_sort(data, count, comp);
    }
    else if (max_depth == 0)
    {
        heap_sort(data, count, comp);
    }
    else
    {
        i64 p = partition(data, count, comp);
        intro_sort(data, p, comp, max_depth - 1);
        intro_sort(data + p, count - p, comp, max_depth - 1);
    }
}

template<typename T, CompFn<T> F>
void intro_sort(T* data, i64 count, F comp)
{
    i64 max_depth = floor(log2(count)) * 2;
    intro_sort(data, count, comp, max_depth);
}

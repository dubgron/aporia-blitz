#pragma once 

#include <chrono>
#include <random>

#include "aporia_debug.hpp"
#include "aporia_memory.hpp"
#include "aporia_string.hpp"
#include "aporia_types.hpp"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(*arr))
#define ARRAY_ZERO(arr) memset(arr, 0, sizeof(arr))
#define INDEX_IN_ARRAY(elem, arr) ((PTR_TO_INT(elem) - PTR_TO_INT(arr)) / sizeof(*elem))

namespace Aporia
{
    String read_entire_file(MemoryArena* arena, String filepath);
    String read_entire_text_file(MemoryArena* arena, String filepath);

    String replace_extension(MemoryArena* arena, String filepath, String ext);
    String extract_filename(String filepath);

    // @NOTE(dubgron): Careful! It's an 'in place' transformation.
    void fix_path_slashes(String* filepath);

    u32 get_hash(String string);

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

    constexpr i32 HUE_RED       = 0;
    constexpr i32 HUE_YELLOW    = 60;
    constexpr i32 HUE_GREEN     = 120;
    constexpr i32 HUE_CYAN      = 180;
    constexpr i32 HUE_BLUE      = 240;
    constexpr i32 HUE_MAGENTA   = 300;

    Color color_from_vec4(f32 r, f32 g, f32 b, f32 a);
    Color hsv_to_rgb(i32 hue, f32 saturation, f32 value);
    void rgb_to_hsv(Color rgb, i32* hue, f32* saturation, f32* value);

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

    // @NOTE(dubgron): Specialization of wrap_around, when we know x >= -y and x < 2y.
    // It is meant to be used as a cheaper alternative when we rougly know the ranges
    // of x and y (e.g. like when incrementing or decrementing an index.)
    template<typename T>
    T wrap_around_once(T x, T y)
    {
        APORIA_ASSERT(y != 0);
        if (x < 0) return x + y;
        if (x >= y) return x - y;
        return x;
    }
}

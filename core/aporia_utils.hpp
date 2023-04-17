#pragma once 

#include <chrono>
#include <random>

#include "aporia_types.hpp"

namespace Aporia
{
    std::string read_file(std::string_view filepath);

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

    Color color_from_vec4(f64 r, f64 g, f64 b, f64 a);
    Color hsv_to_rgb(i64 hue, f64 saturation, f64 value);

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
    T min(T value, T bound)
    {
        return (value > bound) ? bound : value;
    }

    template<typename T> requires std::is_scalar_v<T>
    T max(T value, T bound)
    {
        return (value < bound) ? bound : value;
    }

    template<typename T> requires std::is_scalar_v<T>
    T clamp(T value, T min_bound, T max_bound)
    {
        return min(max(value, min_bound), max_bound);
    }

    f32 degrees_to_radians(f32 angle_in_degrees);
    f32 radians_to_degrees(f32 angle_in_radians);
    i32 unwind_angle(i64 angle);

    i32 random_range(i32 min, i32 max);
    i64 random_range(i64 min, i64 max);
    f32 random_range(f32 min, f32 max);
}

#pragma once

#include <random>
#include <type_traits>

namespace Aporia
{
    template<typename T>
    inline T random(T min, T max)
    {
        static std::mt19937 gen{ std::random_device{}() };

        if constexpr (std::is_integral_v<T>)
            return std::uniform_int_distribution<T>{min, max}(gen);

        if constexpr (std::is_floating_point_v<T>)
            return std::uniform_real_distribution<T>{min, max}(gen);
    }

    template<typename T>
    inline T normal(T mean, T stddev)
    {
        static std::mt19937 gen{ std::random_device{}() };

        return std::normal_distribution<T>{mean, stddev}(gen);
    }
}

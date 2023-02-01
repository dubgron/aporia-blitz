#pragma once

#include <random>
#include <type_traits>

namespace Aporia
{
    namespace impl
    {
        template<typename Dist, typename... Ts>
        inline auto random(Ts... args)
        {
            static std::mt19937 gen{ std::random_device{}() };
            return Dist{ args... }(gen);
        }
    }

    template<typename T> requires std::is_integral_v<T>
    inline T uniform(T min, T max)
    {
        return impl::random<std::uniform_int_distribution<T>>(min, max);
    }

    template<typename T> requires std::is_floating_point_v<T>
    inline T uniform(T min, T max)
    {
        return impl::random<std::uniform_real_distribution<T>>(min, max);
    }

    template<typename T> requires std::is_floating_point_v<T>
    inline T normal(T mean, T stddev)
    {
        return impl::random<std::normal_distribution<T>>(mean, stddev);
    }

    template<typename T> requires std::is_floating_point_v<T>
    inline T exponential(T lambda)
    {
        return impl::random<std::exponential_distribution<T>>(lambda);
    }

    template<typename T = i32> requires std::is_integral_v<T>
    inline T poisson(f32 mean)
    {
        return impl::random<std::poisson_distribution<T>>(mean);
    }

    template<typename T> requires std::is_integral_v<T>
    inline T binomial(T n, f32 p)
    {
        return impl::random<std::binomial_distribution<T>>(n, p);
    }

    inline bool bernoulli(f32 p)
    {
        return impl::random<std::bernoulli_distribution>(p);
    }
}

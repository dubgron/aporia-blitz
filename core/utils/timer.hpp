#pragma once

#include <chrono>
#include <type_traits>

namespace Aporia
{
    using Deltatime = float;

    class Timer final
    {
    public:
        using Clock = std::chrono::steady_clock;
        using TimePoint = Clock::time_point;
        using Duration = Clock::duration;

        using Hours = std::chrono::duration<float, std::ratio<3600>>;
        using Minutes = std::chrono::duration<float, std::ratio<60>>;
        using Seconds = std::chrono::duration<float>;
        using Milliseconds = std::chrono::milliseconds;
        using Microseconds = std::chrono::microseconds;
        using Nanoseconds = std::chrono::nanoseconds;

        template<typename T = Seconds>
        auto reset()
        {
            auto elapsed = get_elapsed_time<T>();
            _start_time = _clock.now();

            return elapsed;
        }

        template<typename T = Seconds>
        auto get_elapsed_time()
        {
            return std::chrono::duration_cast<T>(_clock.now() - _start_time).count();
        }

    private:
        Clock _clock;
        TimePoint _start_time;
    };
}

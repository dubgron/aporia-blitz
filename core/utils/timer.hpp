#pragma once

#include <chrono>
#include <type_traits>

#include "aporia_types.hpp"

namespace Aporia
{
    using Deltatime = f32;

    class Timer final
    {
    public:
        using Clock = std::chrono::steady_clock;
        using TimePoint = Clock::time_point;
        using Duration = Clock::duration;

        using Hours = std::chrono::duration<Deltatime, std::ratio<3600>>;
        using Minutes = std::chrono::duration<Deltatime, std::ratio<60>>;
        using Seconds = std::chrono::duration<Deltatime>;
        using Milliseconds = std::chrono::milliseconds;
        using Microseconds = std::chrono::microseconds;
        using Nanoseconds = std::chrono::nanoseconds;

        Timer() : _start_time(_clock.now()) {}

        template<typename T = Seconds>
        Deltatime reset()
        {
            Deltatime elapsed = get_elapsed_time<T>();
            _start_time = _clock.now();

            return elapsed;
        }

        template<typename T = Seconds>
        Deltatime get_elapsed_time() const
        {
            return std::chrono::duration_cast<T>(_clock.now() - _start_time).count();
        }

    private:
        Clock _clock;
        TimePoint _start_time;
    };
}

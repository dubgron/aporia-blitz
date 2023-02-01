#pragma once

#include <bitset>
#include <cstddef>
#include <type_traits>

#include <magic_enum.hpp>

namespace Aporia
{
    template<typename T> requires std::is_enum_v<T>
    class InputBuffer
    {
        static constexpr u64 Size = magic_enum::enum_count<T>();
        using Buffer = std::bitset<Size>;

    public:
        void update()
        {
            _past_state = _current_state;
        }

        void push_state(T code, bool state)
        {
            u64 index = _index(code);
            _current_state[index] = state;
        }

        bool is_triggered(T code) const
        {
            u64 index = _index(code);
            return _is_triggered(index);
        }

        bool is_pressed(T code) const
        {
            u64 index = _index(code);
            return _is_pressed(index);
        }

        bool is_released(T code) const
        {
            u64 index = _index(code);
            return _is_released(index);
        }

        bool is_any_triggered() const
        {
            for (u64 index = 0; index < Size; ++index)
            {
                if (_is_triggered(index))
                {
                    return true;
                }
            }

            return false;
        }

        bool is_any_pressed() const
        {
            for (u64 index = 0; index < Size; ++index)
            {
                if (_is_pressed(index))
                {
                    return true;
                }
            }

            return false;
        }

        bool is_any_released() const
        {
            for (u64 index = 0; index < Size; ++index)
            {
                if (_is_released(index))
                {
                    return true;
                }
            }

            return false;
        }

    private:
        u64 _index(T code) const
        {
            return magic_enum::enum_index(code).value();
        }

        bool _is_triggered(u64 index) const
        {
            return _current_state.test(index) && !_past_state.test(index);
        }

        bool _is_pressed(u64 index) const
        {
            return _current_state.test(index) && _past_state.test(index);
        }

        bool _is_released(u64 index) const
        {
            return !_current_state.test(index) && _past_state.test(index);
        }

        Buffer _current_state;
        Buffer _past_state;
    };
}

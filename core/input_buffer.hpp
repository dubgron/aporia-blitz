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
        static constexpr size_t Size = magic_enum::enum_count<T>();
        using Buffer = std::bitset<Size>;

    public:
        void update()
        {
            _past_state = _current_state;
        }

        void push_state(T code, bool state)
        {
            size_t index = _index(code);
            _current_state[index] = state;
        }

        bool is_triggered(T code) const
        {
            size_t index = _index(code);
            return _is_triggered(index);
        }

        bool is_pressed(T code) const
        {
            size_t index = _index(code);
            return _is_pressed(index);
        }

        bool is_released(T code) const
        {
            size_t index = _index(code);
            return _is_released(index);
        }

        bool is_any_triggered() const
        {
            for (size_t index = 0; index < Size; ++index)
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
            for (size_t index = 0; index < Size; ++index)
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
            for (size_t index = 0; index < Size; ++index)
            {
                if (_is_released(index))
                {
                    return true;
                }
            }

            return false;
        }

    private:
        size_t _index(T code) const
        {
            return magic_enum::enum_index(code).value();
        }

        bool _is_triggered(size_t index) const
        {
            return _current_state.test(index) && !_past_state.test(index);
        }

        bool _is_pressed(size_t index) const
        {
            return _current_state.test(index) && _past_state.test(index);
        }

        bool _is_released(size_t index) const
        {
            return !_current_state.test(index) && _past_state.test(index);
        }

        Buffer _current_state;
        Buffer _past_state;
    };
}

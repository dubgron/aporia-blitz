#pragma once

#include <bitset>
#include <algorithm>

#include <magic_enum.hpp>

namespace Aporia
{
    template<typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
    class InputBuffer
    {
        using Buffer = std::bitset<magic_enum::enum_count<T>()>;

    public:
        InputBuffer() = default;

        ~InputBuffer()
        {
            delete _incoming_state;
            delete _current_state;
            delete _past_state;
        }

        /* TODO: Add boundary checks */
        void push_state(T code, bool state)
        {
            size_t index = static_cast<size_t>(code);
            (*_incoming_state)[index] = state;
        }

        void update()
        {
            memcpy(_past_state, _incoming_state, sizeof(Buffer));
            std::swap(_past_state, _current_state);
        }

        bool is_triggered(T code) const
        {
            size_t index = static_cast<size_t>(code);
            return _is_triggered(index);
        }

        bool is_pressed(T code) const
        {
            size_t index = static_cast<size_t>(code);
            return _is_pressed(index);
        }

        bool is_released(T code) const
        {
            size_t index = static_cast<size_t>(code);
            return _is_released(index);
        }

        bool is_any_triggered() const
        {
            size_t size = _current_state->size();
            for (size_t i = 0; i < size; ++i)
                if (_is_triggered(i))
                    return true;

            return false;
        }

        bool is_any_pressed() const
        {
            size_t size = _current_state->size();
            for (size_t i = 0; i < size; ++i)
                if (_is_pressed(i))
                    return true;

            return false;
        }

        bool is_any_released() const
        {
            size_t size = _current_state->size();
            for (size_t i = 0; i < size; ++i)
                if (_is_released(i))
                    return true;

            return false;
        }

    private:
        bool _is_triggered(size_t index) const
        {
            return _current_state->test(index) && !_past_state->test(index);
        }

        bool _is_pressed(size_t index) const
        {
            return _current_state->test(index) && _past_state->test(index);
        }

        bool _is_released(size_t index) const
        {
            return !_current_state->test(index) && _past_state->test(index);
        }


        Buffer* _incoming_state = new Buffer(false);
        Buffer* _current_state = new Buffer(false);
        Buffer* _past_state = new Buffer(false);
    };
}

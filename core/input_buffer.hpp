#pragma once

#include <bitset>
#include <algorithm>

#include <magic_enum.hpp>

#include "logger.hpp"
#include "platform.hpp"

namespace Aporia
{
    template<typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
    class APORIA_API InputBuffer
    {
        using Buffer = std::bitset<magic_enum::enum_count<T>()>;

    public:
        InputBuffer(const std::shared_ptr<Logger>& logger)
            : _logger(logger)
        {
        }

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
            return _current_state->test(index) && !_past_state->test(index);
        }

        bool is_pressed(T code) const
        {
            size_t index = static_cast<size_t>(code);
            return _current_state->test(index) && _past_state->test(index);
        }

        bool is_released(T code) const
        {
            size_t index = static_cast<size_t>(code);
            return !_current_state->test(index) && _past_state->test(index);
        }

    private:
        Buffer* _incoming_state = new Buffer(false);
        Buffer* _current_state = new Buffer(false);
        Buffer* _past_state = new Buffer(false);

        std::shared_ptr<Logger> _logger;
    };
}

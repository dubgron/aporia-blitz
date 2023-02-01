#include "animation.hpp"

#include <algorithm>
#include <utility>

#include "aporia_types.hpp"

namespace Aporia
{
    Animation::Animation(std::string name, std::vector<AnimationFrame>&& frames)
        : _name( std::move(name) ), _frames( std::move(frames) ), _frames_amount(_frames.size())
    {
        for (const auto& frame : _frames)
        {
            _length += frame.duration;
        }
    }

    void Animation::play()
    {
        _current_frame = 0;
        _increment = [](u64 curr_frame, u64 frame_count){ return (curr_frame + 1) % frame_count; };
        _timer.reset();
    }

    void Animation::play_once()
    {
        _current_frame = 0;
        _increment = [](u64 curr_frame, u64 frame_count){ return std::min(curr_frame + 1, frame_count - 1); };
        _timer.reset();
    }

    const Texture& Animation::update()
    {
        if (_timer.get_elapsed_time() > _frames[_current_frame].duration)
        {
            _current_frame = _increment(_current_frame, _frames_amount);
            _timer.reset();
        }

        return _frames[_current_frame].texture;
    }
}

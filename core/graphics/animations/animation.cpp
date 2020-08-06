#include "animation.hpp"

#include <algorithm>
#include <utility>

namespace Aporia
{
    Animation::Animation(std::string name, std::vector<AnimationFrame>&& frames)
        : _name(std::move(name)), _frames(std::move(frames)), _frames_amount(_frames.size())
    {
        for (const auto& frame : _frames)
            _length += frame.duration;
    }

    void Animation::play()
    {
        _current_frame = 0;
        _increment = [=](){ return (_current_frame + 1) % _frames_amount; };
        _timer.reset();
    }

    void Animation::play_once()
    {
        _current_frame = 0;
        _increment = [=](){ return std::min(_current_frame + 1, _frames_amount - 1); };
        _timer.reset();
    }

    const Texture& Animation::update()
    {
        if (_timer.get_elapsed_time() > _frames[_current_frame].duration)
        {
            _current_frame = _increment();
            _timer.reset();
        }

        return _frames[_current_frame].texture;
    }
}

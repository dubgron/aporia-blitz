#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "components/texture.hpp"
#include "utils/timer.hpp"

namespace Aporia
{
    struct AnimationFrame final
    {
        Texture texture;
        f32 duration = 0.f;
    };

    class Animation final
    {
    public:
        Animation(std::string name, std::vector<AnimationFrame>&& frames);

        void play();
        void play_once();
        const Texture& update();

        const std::string& get_name() const                     { return _name; }
        const std::vector<AnimationFrame>& get_frames() const   { return _frames; }
        const AnimationFrame& get_current_frame() const         { return _frames[_current_frame]; }
        u64 get_current_frame_index() const                     { return _current_frame; }
        u64 get_frames_amount() const                           { return _frames_amount; }
        f32 get_length() const                                  { return _length; }
        bool is_finished() const                                { return _current_frame == _frames_amount - 1; }

    private:
        std::string _name;
        std::vector<AnimationFrame> _frames;

        f32 _length = 0;
        u64 _frames_amount = 0;

        u64 _current_frame = 0;
        std::function<u64(u64, u64)> _increment = [](u64, u64){ return 0; };

        Timer _timer;
    };
}

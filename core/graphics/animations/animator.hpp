#pragma once

#include <functional>
#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "graphics/animations/animation.hpp"

namespace Aporia
{
    struct Sprite;

    class Animator final
    {
    public:
        Animator(std::string name = "None", Sprite* sprite = nullptr);

        void add_animation(Animation&& animation);
        void play(const std::string& name);
        void play_once(const std::string& name);
        void update();

        void queue(const std::string& name);
        void after_queue(const std::string& name);
        void clear_queue();

        const Animation& get_animation(const std::string& name) const;

    private:
        void _update_queue();

    private:
        std::string _name;
        std::unordered_map<std::string, Animation> _animations;
        std::string _current_animation = "default";

        std::queue<std::string_view> _queue;
        std::string_view _afterqueue = "";

        Sprite* _sprite;
    };
}

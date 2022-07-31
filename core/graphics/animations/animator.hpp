#pragma once

#include <cstddef>
#include <functional>
#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "logger.hpp"
#include "graphics/animations/animation.hpp"
#include "graphics/drawables/sprite.hpp"

namespace Aporia
{
    class Animator final
    {
    public:
        Animator(Logger& logger, std::string name, Sprite& sprite);

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

        Logger& _logger;

        std::string _name;
        std::unordered_map<std::string, Animation> _animations;
        std::string _current_animation = "default";

        std::queue<std::string_view> _queue;
        std::string_view _afterqueue = "";

        Sprite& _sprite;
    };
}

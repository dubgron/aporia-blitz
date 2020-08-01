#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "logger.hpp"
#include "graphics/sprite.hpp"
#include "graphics/animations/animation.hpp"

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

        const Animation& get_animation(const std::string& name) const;

    private:
        Logger& _logger;

        std::string _name;
        std::unordered_map<std::string, Animation> _animations;
        std::string _current_animation = "default";

        Sprite& _sprite;
    };
}

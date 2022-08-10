#pragma once

#include <string>
#include <unordered_map>

#include "graphics/animations/animation.hpp"

namespace Aporia
{
    class EventManager;
    class Logger;
    class TextureManager;
    struct AnimationConfig;

    class AnimationManager final
    {
    public:
        AnimationManager(Logger& logger, EventManager& event_manager, const TextureManager& textures, const AnimationConfig& config);

        Animation get_animation(const std::string& name) const;

    private:
        void _load();

    private:
        Logger& _logger;

        std::unordered_map<std::string, Animation> _animations;

        const TextureManager& _textures;
        const AnimationConfig& _config;
    };
}

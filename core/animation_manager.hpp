#pragma once

#include <string>
#include <unordered_map>

#include "event_manager.hpp"
#include "logger.hpp"
#include "texture_manager.hpp"
#include "configs/animation_config.hpp"
#include "graphics/animations/animation.hpp"

namespace Aporia
{
    class AnimationManager final
    {
    public:
        AnimationManager(Logger& logger, EventManager& event_manager, const TextureManager& textures, const AnimationConfig& config);

        Animation get_animation(const std::string& name) const;

    private:
        Logger& _logger;

        std::unordered_map<std::string, Animation> _animations;

        const TextureManager& _textures;
        const AnimationConfig& _config;

        void _load();
    };
}

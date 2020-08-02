#pragma once

#include <string>
#include <unordered_map>

#include "logger.hpp"
#include "texture_manager.hpp"
#include "configs/animation_config.hpp"
#include "graphics/animations/animation.hpp"

namespace Aporia
{
    class AnimationManager final
    {
    public:
        AnimationManager(Logger& logger, const TextureManager& textures, const AnimationConfig& config);

        Animation get_animation(const std::string& name) const;

    private:
        Logger& _logger;

        std::unordered_map<std::string, Animation> _animations;

        const TextureManager& _textures;
        const AnimationConfig& _config;
    };
}

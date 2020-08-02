#include "animation_manager.hpp"

#include <utility>
#include <vector>

namespace Aporia
{
    AnimationManager::AnimationManager(Logger& logger, const TextureManager& textures, const AnimationConfig& config)
        : _logger(logger), _textures(textures), _config(config)
    {
        _animations.try_emplace("default", Animation{ "default", { { _textures.get_texture("default"), 0.0f } } });

        for (const auto& animation : config.animations)
        {
            const std::string& name = animation.first;
            const auto& frame_configs = _config.animations.at(name);

            std::vector<AnimationFrame> frames;
            frames.reserve(frame_configs.size());

            for (const auto& frame_conf : frame_configs)
            {
                AnimationFrame animation_frame{ _textures.get_texture(frame_conf.texture_name), frame_conf.duration };
                frames.push_back(std::move(animation_frame));
            }

            _animations.try_emplace(name, Animation{ name, std::move(frames) });
        }
    }

    Animation AnimationManager::get_animation(const std::string& name) const
    {
        if (!_animations.contains(name))
        {
            _logger.log(LOG_ERROR) << "No animation named '" << name << "'!";
            return _animations.at("default");
        }
        else
            return _animations.at(name);
    }
}

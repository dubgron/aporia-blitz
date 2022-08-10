#include "animation_manager.hpp"

#include <functional>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "event_manager.hpp"
#include "logger.hpp"
#include "texture_manager.hpp"
#include "configs/animation_config.hpp"
#include "utils/read_file.hpp"

namespace Aporia
{
    AnimationManager::AnimationManager(Logger& logger, EventManager& event_manager, const TextureManager& textures, const AnimationConfig& config)
        : _logger(logger), _textures(textures), _config(config)
    {
        _load();

        event_manager.add_listener<ReloadAnimationConfigEvent>(std::bind(&AnimationManager::_load, this));
    }

    Animation AnimationManager::get_animation(const std::string& name) const
    {
        if (!_animations.contains(name))
        {
            _logger.log(LOG_ERROR) << "No animation named '" << name << "'!";
            return _animations.at("default");
        }
        else
        {
            return _animations.at(name);
        }
    }

    void AnimationManager::_load()
    {
        _animations.clear();
        _animations.try_emplace("default", Animation{ "default", { { _textures.get_texture("default"), 0.0f } } });

        nlohmann::json animation_json = load_json(_config.animations);
        for (const auto& animation : animation_json["animations"])
        {
            std::vector<AnimationFrame> frames;
            frames.reserve(animation["frames"].size());

            for (const auto& frame : animation["frames"])
            {
                AnimationFrame animation_frame{ _textures.get_texture(frame["texture"]), frame["duration"] };
                frames.push_back( std::move(animation_frame) );
            }

            std::string name = animation["name"];
            _animations.try_emplace(name, Animation{ name, std::move(frames) });
        }
    }
}

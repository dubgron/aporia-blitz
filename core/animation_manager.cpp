#include "animation_manager.hpp"

#include <utility>
#include <vector>

#include "utils/read_file.hpp"

namespace Aporia
{
    AnimationManager::AnimationManager(Logger& logger, const TextureManager& textures, const AnimationConfig& config)
        : _logger(logger), _textures(textures), _config(config)
    {
        _animations.try_emplace("default", Animation{ "default", { { _textures.get_texture("default"), 0.0f } } });

        nlohmann::json animation_json = load_json(_config.animations);
        for (const auto& animation : animation_json["animations"])
        {
            std::vector<AnimationFrame> frames;
            frames.reserve(animation["frames"].size());

            for (const auto& frame : animation["frames"])
            {
                Texture texture = _textures.get_texture(frame["texture"]);
                frames.emplace_back(texture, frame["duration"]);
            }

            std::string name = animation["name"];
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

#include "animator.hpp"

#include <algorithm>
#include <utility>

namespace Aporia
{
    Animator::Animator(Logger& logger, std::string name, Sprite& sprite)
        : _logger(logger), _name(std::move(name)), _sprite(sprite)
    {
        _animations.try_emplace("default", Animation{ "default", { { sprite.get_component<Texture>(), 0.0f } } });
    }

    void Animator::add_animation(Animation&& animation)
    {
        std::string name = animation.get_name();

        if (!_animations.contains(name))
            _animations.try_emplace(std::move(name), std::move(animation));
        else
            _logger.log(LOG_WARNING) << "Animation '" << name << "' has already been added!";
    }

    void Animator::play(const std::string& name)
    {
        if (_animations.contains(name))
        {
            _current_animation = name;
            _animations.at(_current_animation).play();
            _logger.log(LOG_DEBUG) << "Playing '" << name << "'";
        }
        else
            _logger.log(LOG_WARNING) << "No animation named '" << name << "'!";
    }

    void Animator::play_once(const std::string& name)
    {
        if (_animations.contains(name))
        {
            _current_animation = name;
            _animations.at(_current_animation).play_once();
            _logger.log(LOG_DEBUG) << "Playing '" << name << "' once";
        }
        else
            _logger.log(LOG_WARNING) << "No animation named '" << name << "'!";
    }

    void Animator::update()
    {
        _sprite.get_component<Texture>() = _animations.at(_current_animation).update();
    }

    const Animation& Animator::get_animation(const std::string& name) const
    {
        if (!_animations.contains(name))
        {
            _logger.log(LOG_WARNING) << "No animation named '" << name << "'!";
            return _animations.at("default");
        }
        else
            return _animations.at(name);
    }
}

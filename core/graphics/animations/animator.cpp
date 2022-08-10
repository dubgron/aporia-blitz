#include "animator.hpp"

#include <algorithm>
#include <utility>

#include "logger.hpp"
#include "graphics/animations/animation.hpp"
#include "graphics/drawables/sprite.hpp"

namespace Aporia
{
    Animator::Animator(Logger& logger, std::string name, Sprite& sprite)
        : _logger(logger), _name( std::move(name) ), _sprite(sprite)
    {
        _animations.try_emplace("default", Animation{ "default", { { sprite.get_component<Texture>(), 0.0f } } });
    }

    void Animator::add_animation(Animation&& animation)
    {
        std::string name = animation.get_name();

        if (!_animations.contains(name))
        {
            _animations.try_emplace(std::move(name), std::move(animation));
        }
        else
        {
            _logger.log(LOG_WARNING) << "Animation '" << name << "' has already been added!";
        }
    }

    void Animator::play(const std::string& name)
    {
        if (!_animations.contains(name))
        {
            _logger.log(LOG_WARNING) << "No animation named '" << name << "'!";
        }
        else if (_queue.empty())
        {
            _current_animation = name;
            _animations.at(_current_animation).play();
        }
    }

    void Animator::play_once(const std::string& name)
    {
        if (!_animations.contains(name))
        {
            _logger.log(LOG_WARNING) << "No animation named '" << name << "'!";
        }
        else if (_queue.empty())
        {
            _current_animation = name;
            _animations.at(_current_animation).play_once();
            _logger.log(LOG_DEBUG) << "Playing '" << name << "' once";
        }
    }

    void Animator::update()
    {
        _update_queue();

        _sprite.get_component<Texture>() = _animations.at(_current_animation).update();
    }

    void Animator::queue(const std::string& name)
    {
        if (_animations.contains(name))
        {
            _queue.push(_animations.at(name).get_name());
        }
    }

    void Animator::after_queue(const std::string& name)
    {
        if (_animations.contains(name))
        {
            _afterqueue = _animations.at(name).get_name();
        }
    }

    void Animator::clear_queue()
    {
        _queue = {};
        _afterqueue = "";
    }

    const Animation& Animator::get_animation(const std::string& name) const
    {
        if (!_animations.contains(name))
        {
            _logger.log(LOG_WARNING) << "No animation named '" << name << "'!";
            return _animations.at("default");
        }
        else
        {
            return _animations.at(name);
        }
    }

    void Animator::_update_queue()
    {
        if (!_animations.at(_current_animation).is_finished())
        {
            return;
        }

        if (!_queue.empty())
        {
            _current_animation = _queue.front();
            _animations.at(_current_animation).play_once();
            _logger.log(LOG_DEBUG) << "Playing '" << _current_animation << "' from a queue (size = " << _queue.size() << ")";
            _queue.pop();
        }
        else if (_afterqueue != "")
        {
            _current_animation = _afterqueue;
            _animations.at(_current_animation).play();
            _logger.log(LOG_DEBUG) << "Playing '" << _current_animation << "' after a queue";
            _afterqueue = "";
        }
    }
}

#include "animator.hpp"

#include <algorithm>
#include <utility>

#include "common.hpp"
#include "graphics/drawables/sprite.hpp"

namespace Aporia
{
    Animator::Animator(std::string name, Sprite& sprite)
        : _name( std::move(name) ), _sprite(sprite)
    {
        _animations.try_emplace("default", Animation{ "default", { { sprite.texture, 0.f } } });
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
            APORIA_LOG(Warning, "Animation '{}' has already been added!", name);
        }
    }

    void Animator::play(const std::string& name)
    {
        if (!_animations.contains(name))
        {
            APORIA_LOG(Warning, "No animation named '{}'!", name);
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
            APORIA_LOG(Warning, "No animation named '{}'!", name);
        }
        else if (_queue.empty())
        {
            _current_animation = name;
            _animations.at(_current_animation).play_once();
            APORIA_LOG(Debug, "Playing '{}' once!", name);
        }
    }

    void Animator::update()
    {
        _update_queue();

        _sprite.texture = _animations.at(_current_animation).update();
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
            APORIA_LOG(Warning, "No animation named '{}'!", name);
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
            APORIA_LOG(Debug, "Playing '{}' from a queue (size = {})", _current_animation, _queue.size());
            _queue.pop();
        }
        else if (_afterqueue != "")
        {
            _current_animation = _afterqueue;
            _animations.at(_current_animation).play();
            APORIA_LOG(Debug, "Playing '{}' after a queue", _current_animation);
            _afterqueue = "";
        }
    }
}

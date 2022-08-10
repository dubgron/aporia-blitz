#pragma once

#include <string>
#include <utility>

#include "utils/timer.hpp"

namespace Aporia
{
    class Logger;
    class InputManager;
    class SceneManager;
    class Renderer;

    class Scene
    {
    public:
        Scene(Logger& logger, std::string name)
            : _logger(logger), _name( std::move(name) ) {}

        virtual void on_load() {};
        virtual void on_input(const InputManager& inputs) {};
        virtual void on_scene_transition(SceneManager& scenes) {};
        virtual void on_update(Deltatime dt) {};
        virtual void on_draw(Renderer& renderer) {};
        virtual void on_unload() {};

        const std::string& get_name() const { return _name; }

    protected:
        Logger& _logger;
        std::string _name;
    };
}

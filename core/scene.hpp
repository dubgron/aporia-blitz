#pragma once

#include <string>
#include <utility>

#include "utils/timer.hpp"

namespace Aporia
{
    class SceneManager;
    class Renderer;

    class Scene
    {
    public:
        Scene(std::string name)
            : _name( std::move(name) )
        {
        }

        virtual void on_load() {};
        virtual void on_scene_transition(SceneManager& scenes) {};
        virtual void on_update(Deltatime dt) {};
        virtual void on_draw(Renderer& renderer) {};
        virtual void on_unload() {};

        const std::string& get_name() const { return _name; }

    protected:
        std::string _name;
    };
}

#pragma once

#include <string>
#include <utility>

#include "logger.hpp"
#include "graphics/renderer.hpp"

namespace Aporia
{
    class Scene
    {
    public:
        Scene(Logger& logger, std::string name)
            : _logger(logger), _name(std::move(name)) {}

        virtual void on_load() {};
        virtual void on_draw(Renderer& renderer) {};
        virtual void on_unload() {};

        const std::string& get_name() const { return _name; }

    protected:
        Logger& _logger;
        std::string _name;
    };
}

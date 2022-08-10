#pragma once

#include <string>
#include <utility>

namespace Aporia
{
    class Logger;

    class Layer
    {
    public:
        Layer(Logger& logger, std::string name)
            : _logger(logger), _name( std::move(name) ) {}

        virtual void on_attach() {};
        virtual void on_detach() {};
        virtual void on_update() {};

        const std::string& get_name() const { return _name; }

    protected:
        Logger& _logger;
        std::string _name;
    };
}

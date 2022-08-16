#pragma once

#include <string>
#include <utility>

namespace Aporia
{
    class Layer
    {
    public:
        Layer(std::string name)
            : _name( std::move(name) ) {}

        virtual void on_attach() {};
        virtual void on_detach() {};
        virtual void on_update() {};

        const std::string& get_name() const { return _name; }

    protected:
        std::string _name;
    };
}

#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "layers/layer.hpp"

namespace Aporia
{
    class Logger;

    class LayerStack final
    {
        using Container = std::vector<std::reference_wrapper<Layer>>;

    public:
        LayerStack(Logger& logger);

        void push_layer(Layer& layer);
        void pop_layer(Layer& layer);

        void push_overlay(Layer& overlay);
        void pop_overlay(Layer& overlay);

        Container::iterator begin()                         { return _layers.begin(); }
        Container::iterator end()                           { return _layers.end(); }
        Container::reverse_iterator rbegin()                { return _layers.rbegin(); }
        Container::reverse_iterator rend()                  { return _layers.rend(); }

        Container::const_iterator begin() const             { return _layers.begin(); }
        Container::const_iterator end() const               { return _layers.end(); }
        Container::const_reverse_iterator rbegin() const    { return _layers.rbegin(); }
        Container::const_reverse_iterator rend() const      { return _layers.rend(); }

    private:
        Logger& _logger;

        Container _layers;
        uint32_t _overlay_index = 0;
    };
}

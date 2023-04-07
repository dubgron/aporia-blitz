#include "layer_stack.hpp"

#include <algorithm>
#include <memory>

namespace Aporia
{
    void LayerStack::push_layer(Layer& layer)
    {
        const auto end = _layers.begin() + _overlay_index;
        auto find = [&layer](const auto& ref){ return std::addressof(ref.get()) == std::addressof(layer); };

        if (std::find_if(_layers.begin(), end, find) == _layers.end())
        {
            layer.on_attach();
            _layers.emplace(_layers.begin() + _overlay_index++, layer);
        }
        else
        {
            APORIA_LOG(Warning, "Layer '{}' has already been pushed!", layer.get_name());
        }
    }

    void LayerStack::push_overlay(Layer& overlay)
    {
        const auto begin = _layers.begin() + _overlay_index;
        auto find = [&overlay](const auto& ref){ return std::addressof(ref.get()) == std::addressof(overlay); };

        if (std::find_if(begin, _layers.end(), find) == _layers.end())
        {
            overlay.on_attach();
            _layers.emplace_back(overlay);
        }
        else
        {
            APORIA_LOG(Warning, "Overlay '{}' has already been pushed!", overlay.get_name());
        }
    }

    void LayerStack::pop_layer(Layer& layer)
    {
        const auto end = _layers.begin() + _overlay_index;

        auto find = [&layer](const auto& ref){ return std::addressof(ref.get()) == std::addressof(layer); };
        auto remove = std::remove_if(_layers.begin(), end, find);

        if (remove != end)
        {
            layer.on_detach();
            _layers.erase(remove);
            --_overlay_index;
        }
        else
        {
            APORIA_LOG(Warning, "No layer named '{}'!", layer.get_name());
        }
    }

    void LayerStack::pop_overlay(Layer& overlay)
    {
        const auto begin = _layers.begin() + _overlay_index;

        auto find = [&overlay](const auto& ref){ return std::addressof(ref.get()) == std::addressof(overlay); };
        auto remove = std::remove_if(begin, _layers.end(), find);

        if (remove != _layers.end())
        {
            overlay.on_detach();
            _layers.erase(remove);
        }
        else
        {
            APORIA_LOG(Warning, "No overlay named '{}'!", overlay.get_name());
        }
    }
}
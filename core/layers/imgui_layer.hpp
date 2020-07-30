#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "logger.hpp"
#include "window.hpp"
#include "layers/layer.hpp"

namespace Aporia
{
    class ImGuiLayer final : public Layer
    {
    public:
        ImGuiLayer(Logger& logger, Window& window);

        void on_attach() override;
        void on_detach() override;

        void update(sf::Time delta_time);
        void display();

    private:
        sf::RenderWindow& _window;
    };
}

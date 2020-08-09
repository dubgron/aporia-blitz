#pragma once

#include "logger.hpp"
#include "window.hpp"
#include "layers/layer.hpp"
#include "utils/timer.hpp"

namespace Aporia
{
    class ImGuiLayer final : public Layer
    {
    public:
        ImGuiLayer(Logger& logger, Window& window);

        void on_attach() override;
        void on_detach() override;

        void begin();
        void end();

    private:
        Window& _window;
    };
}

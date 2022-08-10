#pragma once

#include "layers/layer.hpp"

namespace Aporia
{
    class Logger;
    class Window;

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

#pragma once

#include <string>
#include <memory>

#include "aporia_config.hpp"
#include "aporia_utils.hpp"
#include "aporia_window.hpp"
#include "aporia_world.hpp"
#include "graphics/camera_controller.hpp"
#include "layers/imgui_layer.hpp"
#include "layers/layer_stack.hpp"

namespace Aporia
{
    class Game
    {
        /* Defined by Client */
        friend std::unique_ptr<Game> create_game();

        friend void main_loop(void* game_ptr);

    public:
        Game(const std::string& config_file);
        virtual ~Game();

        virtual void on_init() {};
        virtual void on_update(Deltatime dt) {};
        virtual void on_terminate() {};

        void run();
        void main_loop();

    protected:
        CameraController _camera;

        Window _window;

        LayerStack _layer_stack;
        ImGuiLayer _imgui_layer;

        World _world;

        Deltatime _dt = 0.f;

    private:
        Timer _timer;
    };
}

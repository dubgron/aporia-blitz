#pragma once

#include <string>
#include <memory>

#include "aporia_config.hpp"
#include "font_manager.hpp"
#include "scene.hpp"
#include "scene_manager.hpp"
#include "window.hpp"
#include "world.hpp"
#include "graphics/camera_controller.hpp"
#include "layers/imgui_layer.hpp"
#include "layers/layer_stack.hpp"
#include "utils/timer.hpp"

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
        SceneManager _scenes;
        FontManager _fonts;

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

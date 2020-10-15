#pragma once

#include <string>
#include <memory>

#include "animation_manager.hpp"
#include "camera_controller.hpp"
#include "config_manager.hpp"
#include "event_manager.hpp"
#include "input_manager.hpp"
#include "logger.hpp"
#include "scene_manager.hpp"
#include "texture_manager.hpp"
#include "window.hpp"
#include "graphics/renderer.hpp"
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

    protected:
        Logger _logger;

        ConfigManager _configs;
        EventManager _events;

        Window _window;

        InputManager _inputs;
        SceneManager _scenes;
        TextureManager _textures;
        AnimationManager _animations;

        CameraController _camera;
        Renderer _renderer;

        LayerStack _layer_stack;
        ImGuiLayer _imgui_layer;

        Deltatime _dt;

    private:
        Timer _timer;
    };
}

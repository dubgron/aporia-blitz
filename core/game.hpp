#pragma once

#include <string>
#include <memory>

#include "camera_controller.hpp"
#include "config_manager.hpp"
#include "event_manager.hpp"
#include "input_manager.hpp"
#include "logger.hpp"
#include "texture_manager.hpp"
#include "window.hpp"
#include "graphics/renderer.hpp"
#include "scene/imgui_layer.hpp"
#include "scene/layer_stack.hpp"

namespace Aporia
{
    class Game
    {
        /* Defined by Client */
        friend std::unique_ptr<Game> create_game();

    public:
        Game(const std::string& config_file);
        virtual ~Game();

        virtual void on_init() {};
        virtual void on_update() {};
        virtual void on_terminate() {};

        void run();

    protected:
        Logger _logger;

        ConfigManager _configs;
        EventManager _events;
        InputManager _inputs;
        TextureManager _textures;

        CameraController _camera;
        Renderer _renderer;
        Window _window;

        LayerStack _layer_stack;
        ImGuiLayer _imgui_layer;
    };
}

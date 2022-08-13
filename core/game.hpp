#pragma once

#include <string>
#include <memory>

#include "animation_manager.hpp"
#include "common.hpp"
#include "config_manager.hpp"
#include "event_manager.hpp"
#include "font_manager.hpp"
#include "input_manager.hpp"
#include "scene.hpp"
#include "scene_manager.hpp"
#include "shader_manager.hpp"
#include "texture_manager.hpp"
#include "window.hpp"
#include "graphics/camera_controller.hpp"
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
        void main_loop();

    protected:
        Logger _logger;

        EventManager _events;
        ConfigManager _configs;

        Window _window;

        InputManager _inputs;
        SceneManager _scenes;
        TextureManager _textures;
        AnimationManager _animations;
        FontManager _fonts;
        ShaderManager _shaders;

        CameraController _camera;
        Renderer _renderer;

        LayerStack _layer_stack;
        ImGuiLayer _imgui_layer;

        Deltatime _dt = 0.0f;

    private:
        Timer _timer;
    };
}

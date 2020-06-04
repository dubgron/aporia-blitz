#pragma once

#include <string>
#include <memory>
#include <tuple>

#include <imgui.h>

#include "camera_controller.hpp"
#include "config_manager.hpp"
#include "event_manager.hpp"
#include "input_manager.hpp"
#include "logger.hpp"
#include "platform.hpp"
#include "sprite.hpp"
#include "renderer.hpp"
#include "texture_manager.hpp"
#include "window.hpp"

namespace Aporia
{
    class APORIA_API Game
    {
        /* Defined by Client */
        friend std::unique_ptr<Game> create_game();

    public:
        Game(const std::string& config_file);
        virtual ~Game() = default;

        virtual void on_init() {};
        virtual void on_update(sf::Time deltatime) {};
        virtual void on_terminate() {};

        void run();

        void text(const char* text)
        {
            ImGui::Begin("Debug");
            ImGui::Text("%s", text);
            ImGui::End();
        }

        void text_f(const char* label, float v)
        {
            ImGui::Begin("Debug");
            ImGui::Text("%s: %f", label, v);
            ImGui::End();
        }

        void text_2f(const char* label, float v1, float v2)
        {
            ImGui::Begin("Debug");
            ImGui::Text("%s: (%f, %f)", label, v1, v2);
            ImGui::End();
        }

        void float_slider(const char* label, float* v, float v_min, float v_max)
        {
            ImGui::Begin("Debug");
            ImGui::SliderFloat(label, v, v_min, v_max);
            ImGui::End();
        }

        bool button(const char* label)
        {
            ImGui::Begin("Debug");
            bool result = ImGui::Button(label);
            ImGui::End();

            return result;
        }

        void int_slider(const char* label, int* v, int v_min, int v_max)
        {
            ImGui::Begin("Debug");
            ImGui::SliderInt(label, v, v_min, v_max);
            ImGui::End();
        }

    protected:
        Logger _logger;

        ConfigManager _configs;
        EventManager _events;
        InputManager _inputs;
        TextureManager _textures;

        Renderer _renderer;
        Window _window;
        CameraController _camera;
    };
}

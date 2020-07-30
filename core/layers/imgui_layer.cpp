#include "imgui_layer.hpp"

#include <imgui-SFML.h>

namespace Aporia
{
    ImGuiLayer::ImGuiLayer(Logger& logger, Window& window)
        : Layer(logger, "ImGui Layer"), _window(window.get_native_window()) {}

    void ImGuiLayer::on_attach()
    {
        ImGui::SFML::Init(_window);
    }

    void ImGuiLayer::on_detach()
    {
        ImGui::SFML::Shutdown();
    }

    void ImGuiLayer::update(sf::Time delta_time)
    {
        ImGui::SFML::Update(_window, delta_time);
    }

    void ImGuiLayer::display()
    {
        ImGui::SFML::Render(_window);
    }
}

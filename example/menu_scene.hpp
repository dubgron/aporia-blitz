#pragma once

#include <array>
#include <functional>
#include <utility>

#include <nlohmann/json.hpp>

#include <scene.hpp>

class MenuScene : public Aporia::Scene
{
    using SceneTransition = std::function<void(int)>;

public:
    MenuScene(Aporia::Logger& logger, const SceneTransition& enter, Aporia::Sprite menu)
        : Aporia::Scene(logger, "menu"), enter(enter), menu(menu)
    {
        ImGui::GetIO().Fonts->AddFontDefault();
        font_menu = ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/bank-gothic-light-bt.ttf", 64);
        font_score = ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/bank-gothic-light-bt.ttf", 36);
    }

    void on_load() override
    {
        using json = nlohmann::json;

        std::string data = Aporia::read_file("game.json");
        json config = json::parse(data);

        record = config["record"];
    }

    void on_input(const Aporia::InputManager& inputs) override
    {
        int new_selected = selected;

        if (inputs.is_key_triggered(Aporia::Keyboard::W) || inputs.is_key_triggered(Aporia::Keyboard::Up))
            new_selected = std::max<int>(selected - 1, 0);

        if (inputs.is_key_triggered(Aporia::Keyboard::S) || inputs.is_key_triggered(Aporia::Keyboard::Down))
            new_selected = std::min<int>(selected + 1, options.size() - 1);

        if (selected != new_selected)
        {
            _logger.log(LOG_DEBUG) << options[selected] << " -> " << options[new_selected];
            selected = new_selected;
        }

        if (inputs.is_key_triggered(Aporia::Keyboard::Space) || inputs.is_key_triggered(Aporia::Keyboard::Enter))
        {
            _logger.log(LOG_DEBUG) << "Entered '" << options[selected] << "'";
            enter(selected);
        }
    }

    void on_update(Aporia::Deltatime dt) override
    {
    }

    void on_draw(Aporia::Renderer& renderer) override
    {
        ImGuiWindowFlags flags = 0;
        flags |= ImGuiWindowFlags_NoBackground;
        flags |= ImGuiWindowFlags_NoTitleBar;
        flags |= ImGuiWindowFlags_NoResize;
        flags |= ImGuiWindowFlags_NoMove;

        ImGui::Begin("Menu", (bool*)0, flags);
        {
            ImGui::PushFont(font_menu);
            for (int i = 0; i < options.size(); ++i)
            {
                const std::string& option = options[i];
                ImGui::PushStyleColor(ImGuiCol_Text, i == selected ? ImVec4{ 0.92549f, 0.109804f, 0.109804f, 1.0f } : ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f });
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() / 2.0f - ImGui::CalcTextSize(option.c_str()).x / 2.0f);
                ImGui::Text(option.c_str());
                ImGui::PopStyleColor();
                ImGui::Dummy(ImVec2(0.0f, 75.0f));
            }
            ImGui::PopFont();
        }
        ImGui::End();

        if (record > 0)
        {
            ImGui::Begin("Record", (bool*)0, flags);
            {
                ImGui::PushFont(font_score);
                ImGui::Text("Your best score: %.3fs!", record / 1000.0f);
                ImGui::PopFont();
            }
            ImGui::End();
        }

        renderer.draw(menu);
    }

    void on_unload() override
    {
    }

private:
    int selected = 0;
    std::array<std::string, 3> options = { "Play", "Credits", "Exit" };

    uint32_t record;

    Aporia::Sprite menu;

    SceneTransition enter;

    ImFont* font_menu;
    ImFont* font_score;
};

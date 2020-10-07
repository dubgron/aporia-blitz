#pragma once

#include <imgui.h>
#include <nlohmann/json.hpp>

#include <scene.hpp>

class EndScene : public Aporia::Scene
{
    using SceneTransition = std::function<void(int)>;

public:
    EndScene(Aporia::Logger& logger, const SceneTransition& exit, Aporia::Sprite end)
        : Aporia::Scene(logger, "end"), exit(exit), end(std::move(end))
    {
        font1 = ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/bank-gothic-light-bt.ttf", 64);
        font2 = ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/bank-gothic-light-bt.ttf", 36);
    }

    void on_load() override
    {
        using json = nlohmann::json;

        std::string data = Aporia::read_file("game.json");
        json config = json::parse(data);

        score = config["last_score"];
        record = config["record"];
    }

    void on_input(const Aporia::InputManager& inputs) override
    {
        if (inputs.is_any_key_triggered())
            exit(1);
    }

    void on_update(Aporia::Deltatime dt) override
    {
    }

    void on_draw(Aporia::Renderer& renderer) override
    {
        renderer.draw(end);

        ImGuiWindowFlags flags = 0;
        flags |= ImGuiWindowFlags_NoBackground;
        flags |= ImGuiWindowFlags_NoTitleBar;
        flags |= ImGuiWindowFlags_NoResize;
        flags |= ImGuiWindowFlags_NoMove;

        ImGui::Begin("Score", (bool*)0, flags);
        {
            ImGui::PushFont(font1);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() / 2.0f - (ImGui::CalcTextSize("0").x * (std::log10(score/ 1000.0f) + 1) + ImGui::CalcTextSize(".000s!").x) / 2.0f);
            ImGui::Text("%.3fs!", score / 1000.0f);
            ImGui::PopFont();

            if (score > record)
            {
                ImGui::PushFont(font2);
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() / 2.0f - ImGui::CalcTextSize("New record!").x / 2.0f);
                ImGui::Text("New record!");
                ImGui::PopFont();
            }
        }
        ImGui::End();
    }

    void on_unload() override
    {
        using json = nlohmann::json;

        std::string data = Aporia::read_file("game.json");
        json config = json::parse(data);

        config["record"] = score;

        std::ofstream file("game.json");
        file << config.dump(2);
    }

private:
    SceneTransition exit;
    Aporia::Sprite end;

    uint32_t score;
    uint32_t record;

    ImFont* font1;
    ImFont* font2;
};

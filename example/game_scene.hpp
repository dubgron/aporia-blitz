#pragma once

#include <imgui.h>
#include <PerlinNoise.hpp>

#include <camera_controller.hpp>
#include <scene.hpp>
#include <utils/timer.hpp>

#include "cannon.hpp"
#include "player.hpp"

class GameScene : public Aporia::Scene
{
    using SceneTransition = std::function<void(int)>;

public:
    GameScene(Aporia::Logger& logger, const SceneTransition& exit, Aporia::CameraController& camera, Aporia::Sprite heart)
        : Aporia::Scene(logger, "game"), exit(exit), camera(camera), heart(std::move(heart))
    {
        font = ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/bank-gothic-light-bt.ttf", 16);
    }

    void on_load() override
    {
        player.load_config();
        player.init();

        slomo_regen_timer.reset();

        cannon.load_config();
        cannon.init();

        time_passed = 0.0f;
    }

    void on_input(const Aporia::InputManager& inputs) override
    {
        if (player.available && inputs.is_key_triggered(Aporia::Keyboard::Space))
        {
            player.center_position = dir(t) * (player.current_radius + player.next_radius) + player.center_position;
            player.current_radius = player.next_radius;

            t += M_PI;
            player.velocity *= -1.0f;
        }

        if (inputs.is_key_triggered(Aporia::Keyboard::E))
            player.velocity *= -1.0f;

        slomo = inputs.is_key_pressed(Aporia::Keyboard::Q);
    }

    void on_update(Aporia::Deltatime dt) override
    {
        if ((slomo && player.slomo_meter > 0.0f) || player.lives == 0)
        {
            player.slomo_meter = std::max(player.slomo_meter - dt, 0.0f);
            dt /= player.slomo;

            slomo_regen_timer.reset();
        }
        else if (player.slomo_meter < player.slomo_max && slomo_regen_timer.get_elapsed_time() > player.invincible_regen)
            player.slomo_meter = std::min(player.slomo_meter + dt, player.slomo_max);

        time_passed += dt;
        t += dt * player.velocity;

        float noise = perlin.noise1D_0_1(time_passed);
        player.next_radius = noise * player.min_radius + (1.0f - noise) * player.max_radius;

        for (auto& bullet : cannon.bullets)
            bullet.update(dt);

        cannon.update();

        //ImGui::Begin("DEBUG");
        //{
        //    ImGui::Text("%.1f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

        //    if (ImGui::Button("Reload config"))
        //    {
        //        player.load_config();
        //        cannon.load_config();
        //    }

        //    if (ImGui::Button("Hit"))
        //    {
        //        player.lives = std::max(player.lives - 1, 0);
        //        player.invincible_timer.reset();
        //    }

        //    ImGui::Text("Bullet count: %d", cannon.bullets.size());
        //    ImGui::Text("Current stage: %d", cannon.current_stage);
        //    ImGui::Text("Stage time left: %f", cannon.stages[cannon.current_stage].duration - cannon.stage_timer.get_elapsed_time());
        //    ImGui::Text("Shoot time left: %f", 1.0f / cannon.stages[cannon.current_stage].shoot_rate - cannon.shoot_timer.get_elapsed_time());
        //    ImGui::Checkbox("Bullet Hell", &cannon.stages[cannon.current_stage].bullet_hell);

        //    ImGui::Text("You survived: %f seconds", time_passed);
        //    ImGui::Text("Slomo meter: %f seconds", player.slomo_meter);
        //}
        //ImGui::End();

        bool invincible = player.invincible_timer.get_elapsed_time() <= player.invincible_duration;
        if (player.lives == 0 && !invincible)
            exit(3);
        else if (player.lives == 0)
        {
            camera.follow(dir(t) * player.current_radius + player.center_position, dt * 2.0f);
            camera.get_camera().zoom(-dt * 2.0f);
        }
    }

    void on_draw(Aporia::Renderer& renderer) override
    {
        renderer.draw(Aporia::Circle2D{ glm::vec3{ player.center_position, 0.0f }, player.size / 5.0f, Aporia::Colors::Cyan });
        //renderer.draw(Aporia::Circle2D{ glm::vec3{ player.center_position, -0.1f }, player.current_radius, Aporia::Color{ 0, 255, 255, 25 } });

        const uint32_t LINES = 30;
        float angle = M_PI / LINES;
        for (int i = 0; i < LINES; ++i)
        {
            glm::vec3 p_1 = glm::vec3{ dir(angle * i * 2.0f) * player.current_radius + player.center_position, -0.1f };
            glm::vec2 p_2 = dir(angle * (i * 2.0f + 1)) * player.current_radius + player.center_position;

            renderer.draw(Aporia::Line2D{ p_1, p_2, Aporia::Color{ 0, 255, 255, 128 } });
        }

        glm::vec2 new_player_pos = dir(t) * (player.current_radius + player.next_radius) + player.center_position;

        player.available = !(new_player_pos.x + player.next_radius + player.size / 2.0f >= 400 || new_player_pos.x - player.next_radius - player.size / 2.0f <= -400
            || new_player_pos.y + player.next_radius + player.size / 2.0f >= 400 || new_player_pos.y - player.next_radius - player.size / 2.0f <= -400);

        renderer.draw(Aporia::Circle2D{ glm::vec3{ new_player_pos, 0.0f }, player.size / 5.0f, player.available ? Aporia::Colors::Yellow : Aporia::Colors::Red });
        //renderer.draw(Aporia::Circle2D{ glm::vec3{ new_player_pos, -0.1f }, player.next_radius, [](Aporia::Color c){ c.a = 25; return c; }(player.available ? Aporia::Colors::Yellow : Aporia::Colors::Red) });

        for (int i = 0; i < LINES; ++i)
        {
            glm::vec3 p_1 = glm::vec3{ dir(angle * i * 2.0f) * player.next_radius + new_player_pos, -0.1f };
            glm::vec2 p_2 = dir(angle * (i * 2.0f + 1)) * player.next_radius + new_player_pos;

            renderer.draw(Aporia::Line2D{ p_1, p_2, [](Aporia::Color c){ c.a = 128; return c; }(player.available ? Aporia::Colors::Yellow : Aporia::Colors::Red) });
            //renderer.draw(Aporia::Line2D{ p_1, p_2, player.available ? Aporia::Colors::Yellow : Aporia::Colors::Red });
        }


        glm::vec2 player_pos = dir(t) * player.current_radius + player.center_position;

        bool collision = collide();
        bool invincible = player.invincible_timer.get_elapsed_time() <= player.invincible_duration;

        if (collision && !invincible)
        {
            player.lives = std::max(player.lives - 1, 0);
            player.invincible_timer.reset();
        }

        Aporia::Color player_color;
        if (invincible)
            player_color = Aporia::Color{ 200, 50, 50, 100 };
        else
            player_color = Aporia::Color{ 200, 200, 200, 255 };

        renderer.draw(Aporia::Circle2D{ glm::vec3{ player_pos, 0.0f }, player.size, player_color });

        renderer.draw(Aporia::Circle2D{ glm::vec3{ 0.0f, -400.0f, 0.2f}, 40.0f, Aporia::Color{ 200, 200, 200, 255 } });

        for (const auto& bullet : cannon.bullets)
        {
            for (int i = bullet.positions.size() - 1; i >= 0; --i)
            {
                float factor = 1.0f - i / float(bullet.positions.size());
                glm::vec3 position{ bullet.positions[i], 0.1f * factor };
                renderer.draw(Aporia::Circle2D{ position, bullet.size * factor, Aporia::Color{ 0, 255, 0, uint8_t(factor * 255) } });
            }
        }

        for (int i = 0; i < player.lives; ++i)
        {
            auto& transform = heart.get_component<Aporia::Transform2D>();
            transform.position = glm::vec3{ -400.0f + gui_size * (2.0f + 3 * i), 400.0f - gui_size * 2.0f, 0.3f };
            transform.position -= glm::vec3{ transform.origin, 0.0f };

            renderer.draw(heart);
        }
            //renderer.draw(Aporia::Circle2D{ glm::vec3{ -400.0f + gui_size * (2.0f + 3 * i), 400.0f - gui_size * 2.0f, 0.3f }, gui_size, Aporia::Colors::Red });


        renderer.draw(Aporia::Rectangle2D{ glm::vec3{ 400.0f - 1.5f * gui_size - player.slomo_max * 40.0f, 400.0f - 3.5f * gui_size, 0.2f }, player.slomo_max * 40.0f + gui_size, gui_size * 3.0f, Aporia::Color{ 10, 10, 10, 255 } });
        renderer.draw(Aporia::Rectangle2D{ glm::vec3{ 400.0f - gui_size - player.slomo_max * 40.0f, 400.0f - 3.0f * gui_size, 0.3f }, player.slomo_meter * 40.0f, gui_size * 2.0f, Aporia::Color{ 255, 255, 0, 200 } });

        if (player.lives > 0)
        {
            ImGuiWindowFlags flags = 0;
            flags |= ImGuiWindowFlags_NoBackground;
            flags |= ImGuiWindowFlags_NoTitleBar;
            flags |= ImGuiWindowFlags_NoResize;
            flags |= ImGuiWindowFlags_NoMove;

            ImGui::Begin("Slomo", (bool*)0, flags);
            {
                ImGui::PushFont(font);
                ImGui::Text("Slow Motion");
                ImGui::PopFont();
            }
            ImGui::End();
        }
    }

    void on_unload() override
    {
        camera.get_camera().set_zoom(1.0f);
        camera.get_camera().set_position(glm::vec2{ 0.0f });
        _logger.log(LOG_INFO) << "You survived " << time_passed << " seconds";

        using json = nlohmann::json;

        std::string data = Aporia::read_file("game.json");
        json config = json::parse(data);

        config["last_score"] = int(time_passed * 1000);

        std::ofstream file("game.json");
        file << config.dump(2);
    }

private:
    bool collide()
    {
        glm::vec2 player_pos = dir(t) * player.current_radius + player.center_position;
        for (const auto& bullet : cannon.bullets)
        {
            if (bullet.positions.empty())
                continue;

            glm::vec2 bullet_pos = bullet.positions.front();
            if (glm::distance(player_pos, bullet_pos) < player.size * 0.95f + bullet.size)
                return true;
        }

        return false;
    }

    Player player;
    Cannon cannon;
    float time_passed = 0.0f;
    float t = 0.0f;

    SceneTransition exit;
    Aporia::CameraController& camera;

    bool slomo = false;

    Aporia::Timer slomo_regen_timer;

    std::function<glm::vec2(float)> dir = [](float t){ return glm::vec2{ std::cos(t), std::sin(t) }; };

    siv::PerlinNoise perlin;

    float gui_size = 15.0f;
    ImFont* font;

    Aporia::Sprite heart;
};

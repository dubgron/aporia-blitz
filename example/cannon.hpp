#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>

#include <utils/random.hpp>
#include <utils/timer.hpp>

struct BulletConfig
{
    float speed;
    float size;
    uint32_t max_trace;
};

struct Bullet
{
    float speed;
    float size;
    uint32_t max_trace;

    std::vector<glm::vec2> positions;
    glm::vec2 direction;

    Bullet(const BulletConfig& config)
    {
        positions.emplace_back(0.0f, -400.f);

        float t = std::clamp<float>(Aporia::normal<float>(M_PI_2, 0.5f), 0.0f, M_PI);
        direction = glm::vec2{ std::cos(t), std::sin(t) };

        speed = config.speed;
        size = config.size;
        max_trace = config.max_trace;
    }

    void update(Aporia::Deltatime dt)
    {
        if (positions.size() < max_trace)
            positions.emplace_back(0.0f, -400.f);

        positions.front() += direction * dt * speed;
        for (int i = positions.size() - 1; i > 0; --i)
            positions[i] = positions[i - 1];
    }
};

struct Stage
{
    bool bullet_hell;
    float duration;
    float shoot_rate;
};

struct Cannon
{
    std::vector<Stage> stages;

    std::vector<Bullet> bullets;

    size_t current_stage;

    Aporia::Timer stage_timer;
    Aporia::Timer shoot_timer;

    BulletConfig bullet_config;

    void load_config()
    {
        using json = nlohmann::json;

        std::string data = Aporia::read_file("game.json");
        json config = json::parse(data);

        auto cannon = config["cannon"];

        for (const auto& stage_config : cannon["stages"])
        {
            Stage stage;
            stage.bullet_hell = stage_config["bullet_hell"];
            stage.duration = stage_config["duration"];
            stage.shoot_rate = stage_config["shoot_rate"];

            stages.push_back(std::move(stage));
        }

        auto bullet = config["bullet"];

        bullet_config.speed = bullet["speed"];
        bullet_config.size = bullet["size"];
        bullet_config.max_trace = bullet["max_trace"];
    }

    void init()
    {
        current_stage = 0;
        stage_timer.reset();
        shoot_timer.reset();
        bullets.clear();
    }

    void gen_bullet()
    {
        Bullet bullet(bullet_config);
        bullets.push_back(std::move(bullet));
    }

    void update()
    {
        if (stages[current_stage].duration > 0 && stage_timer.get_elapsed_time() > stages[current_stage].duration)
        {
            current_stage++;
            stage_timer.reset();
        }

        if (shoot_timer.get_elapsed_time() > 1.0f / stages[current_stage].shoot_rate)
        {
            gen_bullet();
            shoot_timer.reset();
        }

        if (!stages[current_stage].bullet_hell)
        {
            auto remove = std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& bullet)
                {
                    glm::vec2 last = bullet.positions.back();
                    return std::abs(last.x) > 400.0f || std::abs(last.y) > 400.0f;
                });

            if (remove != bullets.end())
                bullets.erase(remove);
        }
        else
        {
            for (auto& bullet : bullets)
            {
                glm::vec2 first = bullet.positions.front();
                if (std::abs(first.x) > 400.f)
                    bullet.direction.x *= -1.0f;

                if (std::abs(first.y) > 400.f)
                    bullet.direction.y *= -1.0f;
            }
        }
    }
};

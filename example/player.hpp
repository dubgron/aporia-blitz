#pragma once

#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>
#include <PerlinNoise.hpp>

#include <utils/random.hpp>
#include <utils/read_file.hpp>
#include <utils/timer.hpp>

struct Player
{
    float speed;
    float size;
    float min_radius;
    float max_radius;
    float slomo;
    float slomo_max;
    int lives;
    Aporia::Deltatime invincible_duration;
    Aporia::Deltatime invincible_regen;

    float velocity;
    glm::vec2 center_position;

    float current_radius;
    float next_radius;

    bool available;

    Aporia::Timer invincible_timer;

    float slomo_meter;

    void load_config()
    {
        using json = nlohmann::json;

        std::string data = Aporia::read_file("game.json");
        json config = json::parse(data);

        const auto& player = config["player"];

        speed = player["speed"];
        size = player["size"];
        min_radius = player["min_radius"];
        max_radius = player["max_radius"];
        slomo = player["slomo"];
        slomo_max = player["slomo_max"];
        lives = player["lives"];
        invincible_duration = player["invincible_duration"];
        invincible_regen = player["invincible_regen"];
    }

    void init()
    {
        center_position = glm::vec2{ 0.0f };
        velocity = speed;
        current_radius = Aporia::random(min_radius, max_radius);
        next_radius = Aporia::random(min_radius, max_radius);
        slomo_meter = slomo_max;
    }
};

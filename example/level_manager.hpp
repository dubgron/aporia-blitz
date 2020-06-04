#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <iostream>

struct LevelComponents
{
    char player = '@';
    char animal = '%';
    char food = '$';
    char trap = 't';
    char wall = '#';
    char floor = '.';
};

struct Level
{
    Level() = default;
    Level(std::string level, size_t width, size_t height, const LevelComponents& components)
        : level(std::move(level)), width(width), height(height), components(components)
    {
    };

    char get_cell(size_t x, size_t y) const
    {
        if (x >= width || x < 0 || y >= height || y < 0)
            return 0;

        return level.at(x + y * (width + 1));
    }

    bool is_walkable(size_t x, size_t y) const
    {
        char cell = get_cell(x, y);
        return cell == components.floor || cell == components.animal || cell == components.player;
    }

    sf::Vector2i get_player() const
    {
        auto player = level.find(components.player);
        if (player != level.npos)
        {
            return sf::Vector2i(player % (width + 1), player / (width + 1));
        }
        else return { -1, -1 };
    }

    sf::Vector2i get_animal() const
    {
        auto animal = level.find(components.animal);
        if (animal != level.npos)
        {
            return sf::Vector2i(animal % (width + 1), animal / (width + 1));
        }
        else return { -1, -1 };
    }

    sf::Vector2i get_food() const
    {
        auto food = level.find(components.food);
        if (food != level.npos)
        {
            return sf::Vector2i(food % (width + 1), food / (width + 1));
        }
        else return { -1, -1 };
    }

    std::string level;
    size_t width = 0;
    size_t height = 0;

    LevelComponents components;
};

class LevelManager
{
public:
    LevelManager() = default;
    LevelManager(std::vector<Level> levels)
        : levels(std::move(levels))
    {
    };

    Level get_level(size_t level) const
    {
        return levels.at(level);
    }

    size_t count() const
    {
        return levels.size();
    }

private:
    std::vector<Level> levels;
};

LevelManager load_levels_from_file(std::string path)
{
    using json = nlohmann::json;

    std::string data = Aporia::read_file(path);
    json game = json::parse(data);

    LevelComponents lc;
    lc.player = game["player"].get<std::string>()[0];
    lc.animal = game["animal"].get<std::string>()[0];
    lc.food = game["food"].get<std::string>()[0];
    lc.trap = game["trap"].get<std::string>()[0];
    lc.wall = game["wall"].get<std::string>()[0];
    lc.floor = game["floor"].get<std::string>()[0];

    std::vector<Level> levels;
    levels.reserve(game["levels"].size());
    for (const auto& lvl : game["levels"])
    {
        size_t width = lvl[0].get<std::string>().size();
        size_t height = lvl.size();
        std::string l = "";
        for (const auto& line : lvl)
        {
            l += line.get<std::string>() + '\n';
        }

        levels.emplace_back(l, width, height, lc);
    }

    return LevelManager(levels);
}
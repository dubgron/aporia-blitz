#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <iostream>

struct LevelComponents
{
    char player = '@';
    char door = '+';
    char key = '$';
    char everything = 'd';
    char nothing = 'n';
    char is = 'i';
    char color1 = 'b';
    char color2 = 'p';
    char movable = 'o';
    char wall = '#';
    char floor = '.';
    char empty = ' ';

    size_t color1v = 0x448ee7ff;
    size_t color2v = 0xfd76Beff;
};

struct Level
{
    Level() = default;
    Level(std::string level, size_t width, size_t height, const LevelComponents& components)
        : level(std::move(level)), width(width), height(height), components(components)
    {
    };

    void swap(size_t x1, size_t y1, size_t x2, size_t y2)
    {
        std::swap(level[x1 + y1 * (width + 1)], level[x2 + y2 * (width + 1)]);
    }

    char get_cell(size_t x, size_t y) const
    {
        if (x >= width || x < 0 || y >= height || y < 0)
            return 0;

        return level.at(x + y * (width + 1));
    }

    size_t everything_color() const
    {
        for (size_t i = 0; i < width; ++i)
            for (size_t j = 0; j < height; ++j)
                if (get_cell(i, j) == components.everything)
                    if (get_cell(i + 1, j) == components.is)
                    {
                        if (get_cell(i + 2, j) == components.color1)
                            return components.color1v;
                        else if (get_cell(i + 2, j) == components.color2)
                            return components.color2v;
                    }
                    else if (get_cell(i - 1, j) == components.is)
                    {
                        if (get_cell(i - 2, j) == components.color1)
                            return components.color1v;
                        else if (get_cell(i - 2, j) == components.color2)
                            return components.color2v;
                    }
                    else if (get_cell(i, j + 1) == components.is)
                    {
                        if (get_cell(i, j + 2) == components.color1)
                            return components.color1v;
                        else if (get_cell(i, j + 2) == components.color2)
                            return components.color2v;
                    }
                    else if (get_cell(i, j - 1) == components.is)
                    {
                        if (get_cell(i, j - 2) == components.color1)
                            return components.color1v;
                        else if (get_cell(i, j - 2) == components.color2)
                            return components.color2v;
                    }

        return 0;
    }

    size_t nothing_color() const
    {
        for (size_t i = 0; i < width; ++i)
            for (size_t j = 0; j < height; ++j)
                if (get_cell(i, j) == components.nothing)
                    if (get_cell(i + 1, j) == components.is)
                    {
                        if (get_cell(i + 2, j) == components.color1)
                            return components.color1v;
                        else if (get_cell(i + 2, j) == components.color2)
                            return components.color2v;
                    }
                    else if (get_cell(i - 1, j) == components.is)
                    {
                        if (get_cell(i - 2, j) == components.color1)
                            return components.color1v;
                        else if (get_cell(i - 2, j) == components.color2)
                            return components.color2v;
                    }
                    else if (get_cell(i, j + 1) == components.is)
                    {
                        if (get_cell(i, j + 2) == components.color1)
                            return components.color1v;
                        else if (get_cell(i, j + 2) == components.color2)
                            return components.color2v;
                    }
                    else if (get_cell(i, j - 1) == components.is)
                    {
                        if (get_cell(i, j - 2) == components.color1)
                            return components.color1v;
                        else if (get_cell(i, j - 2) == components.color2)
                            return components.color2v;
                    }

        return 0;
    }

    bool is_movable(size_t x, size_t y) const
    {
        char cell = get_cell(x, y);
        if (cell == 0 ||
            cell == components.door ||
            cell == components.wall ||
            cell == components.floor)
            return false;

        size_t ec = everything_color();
        size_t nc = nothing_color();
        if (cell == components.key)
            return ec != 0 && nc != 0 && ec != nc;

        return true;
    }

    bool is_walkable(size_t x, size_t y) const
    {
        size_t ec = everything_color();
        size_t nc = nothing_color();
        char cell = get_cell(x, y);
        return cell == components.floor || (cell == components.key && !(ec != 0 && nc != 0 && ec != nc));
    }
    
    bool can_be_moved(size_t x, size_t y, size_t x_t, size_t y_t) const
    {
        return is_movable(x, y) && is_walkable(x_t, y_t);
    }

    /** dir: 0 - up
      *      1 - right
      *      2 - down
      *      3 - left
      */
    int move(size_t x, size_t y, short dir)
    {
        size_t xx = x;
        size_t yy = y;
        switch (dir)
        {
            case 0: --yy; break;
            case 1: ++xx; break;
            case 2: ++yy; break;
            case 3: --xx; break;
        }

        if (get_cell(x, y) == components.key && is_movable(x, y) && get_cell(xx, yy) == components.door)
            return 2;

        bool moved = can_be_moved(x, y, xx, yy);
        if (moved)
            swap(x, y, xx, yy);

        return moved;
    }

    sf::Vector2i get_player() const
    {
        auto player = level.find(components.player);
        if (player != level.npos)
        {
            return sf::Vector2i(player % (width + 1), player / (height + 1));
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
        : levels(std::move(levels)) { };

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
    lc.door = game["door"].get<std::string>()[0];
    lc.key = game["key"].get<std::string>()[0];
    lc.everything = game["everything"].get<std::string>()[0];
    lc.nothing = game["nothing"].get<std::string>()[0];
    lc.is = game["is"].get<std::string>()[0];
    lc.color1 = game["color1"].get<std::string>()[0];
    lc.color2 = game["color2"].get<std::string>()[0];
    lc.movable = game["movable"].get<std::string>()[0];
    lc.wall = game["wall"].get<std::string>()[0];
    lc.floor = game["floor"].get<std::string>()[0];
    lc.empty = game["empty"].get<std::string>()[0];

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
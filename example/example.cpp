#include <aporia.hpp>

#include <SFML/Audio.hpp>
#include "level_manager.hpp"
#include <string>
#include <algorithm>
#include <chrono>
#include <cmath>

enum class State
{
    Start,
    Game,
    End
};

class Example : public Aporia::Game
{
public:
    ~Example() {};
    LevelManager levels;
    size_t lvl = 0;

    Level current_level;
    sf::Vector2i player;

    std::map<std::string, sf::SoundBuffer> sounds;
    sf::Sound play;

    std::pair<Level, size_t> save_level;
    bool saved = false;

    State current_state = State::Start;

    int start_status = 0;
    std::unique_ptr<Aporia::Sprite> logo;
    std::unique_ptr<Aporia::Sprite> press;
    std::unique_ptr<Aporia::Sprite> rules;

    std::unique_ptr<Aporia::Sprite> the_end;

    void start_level(size_t level)
    {
        current_level = levels.get_level(level);
    }

    void load_sound(std::string path)
    {
        sf::SoundBuffer sound;
        if (!sound.loadFromFile(path))
            get_logger()->log(LOG_ERROR) << "Unable to load sound " << path << "!";
        path.erase(path.rfind('.'));
        sounds[path] = sound;
    }

    void play_sound(const std::string& name)
    {
        auto sound = sounds.find(name);
        if (sound != sounds.end())
        {
            get_logger()->log(LOG_DEBUG) << "Played " << name;
            play.setBuffer(sound->second);
            play.play();
        }
        else
            get_logger()->log(LOG_ERROR) << "There is no sound named " << name << "!";
    }

    void draw_level()
    {
        size_t size = current_level.level.size();
        for (size_t i = 0; i < size; ++i)
        {
            char c = current_level.level[i];
            if (c == '\n')
                continue;

            sf::Vector2f position = 32.0f * sf::Vector2f(i % (current_level.width + 1), i / (current_level.height + 1));

            size_t everything_color = current_level.everything_color();
            size_t nothing_color = current_level.nothing_color();

            std::string tex;
            if (c == current_level.components.floor)
                tex = "floor";
            else if (c == current_level.components.player)
                tex = "player";
            else if (c == current_level.components.door)
                tex = "door";
            else if (c == current_level.components.everything)
                tex = "everything";
            else if (c == current_level.components.nothing)
                tex = "nothing";
            else if (c == current_level.components.is)
                tex = "is";
            else if (c == current_level.components.color1)
                tex = "color1";
            else if (c == current_level.components.color2)
                tex = "color2";
            else if (c == current_level.components.movable)
                tex = "movable";
            else if (c == current_level.components.wall)
                tex = "wall";
            else if (c == current_level.components.empty)
                tex = "empty";
            else if (c == current_level.components.key)
            {
                if (everything_color != 0 && nothing_color != 0)
                {
                    Aporia::Sprite floor(get_textures()->get_texture("floor"), position);
                    floor.color = sf::Color(everything_color);
                    draw(floor);

                    tex = "key";
                }
                else tex = "floor";
            }

            Aporia::Sprite s(get_textures()->get_texture(tex), position);

            if (everything_color != 0)
            {
                s.color = sf::Color(everything_color);
                if (nothing_color != 0 && tex == "key")
                    s.color = sf::Color(nothing_color);
            }

            draw(s);
        }
    }

    void handle_inputs()
    {
        bool up = get_inputs()->is_key_triggered(Aporia::Keyboard::Up) || get_inputs()->is_key_triggered(Aporia::Keyboard::W);
        bool right = get_inputs()->is_key_triggered(Aporia::Keyboard::Right) || get_inputs()->is_key_triggered(Aporia::Keyboard::D);
        bool down = get_inputs()->is_key_triggered(Aporia::Keyboard::Down) || get_inputs()->is_key_triggered(Aporia::Keyboard::S);
        bool left = get_inputs()->is_key_triggered(Aporia::Keyboard::Left) || get_inputs()->is_key_triggered(Aporia::Keyboard::A);

        bool reset = get_inputs()->is_key_triggered(Aporia::Keyboard::R);
        bool save = get_inputs()->is_key_triggered(Aporia::Keyboard::F1);
        bool load = get_inputs()->is_key_triggered(Aporia::Keyboard::F2);

        int result = 0;

        /* if (get_inputs()->is_key_triggered(Aporia::Keyboard::Backspace))
        {
            if (lvl != 0)
                start_level(--lvl);
            else
                current_state = State::Start;
        }
        else if (get_inputs()->is_key_triggered(Aporia::Keyboard::Enter))
        {
            if (lvl != levels.count() - 1)
                start_level(++lvl);
            else
                current_state = State::End;
        } */

        if (save)
        {
            save_level = { current_level, lvl };
            saved = true;
        }
        else if (load && saved)
        {
            current_level = save_level.first;
            lvl = save_level.second;
        }
        else if (reset)
            start_level(lvl);
        else if (up)
        {
            result = current_level.move(player.x, player.y - 1, 0);
            if (current_level.is_walkable(player.x, player.y - 1))
                current_level.swap(player.x, player.y, player.x, player.y - 1);
        }
        else if (right)
        {
            result = current_level.move(player.x + 1, player.y, 1);
            if (current_level.is_walkable(player.x + 1, player.y))
                current_level.swap(player.x, player.y, player.x + 1, player.y);
        }
        else if (down)
        {
            result = current_level.move(player.x, player.y + 1, 2);
            if (current_level.is_walkable(player.x, player.y + 1))
                current_level.swap(player.x, player.y, player.x, player.y + 1);
        }
        else if (left)
        {
            result = current_level.move(player.x - 1, player.y, 3);
            if (current_level.is_walkable(player.x - 1, player.y))
                current_level.swap(player.x, player.y, player.x - 1, player.y);
        }

        if (up || right || down || left)
        {
            play_sound("move");
            get_logger()->log(LOG_DEBUG) << '\n' << current_level.level;
        }

        if (result == 1)
            play_sound("push");

        if (result == 2)
        {
            play_sound("win");
            if (levels.count() == lvl + 1)
                current_state = State::End;
            else
            {
                start_level(++lvl);
            }
        }
    }

    void start()
    {
        double val = (1 + std::sin(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 100) / 2) / 2;
        
        switch (start_status)
        {
            case 0:
                press->color = sf::Color(255, 255, 255, 255 * val);
                draw(*logo);
                draw(*press); 
                break;
            case 1:
                draw(*rules); 
                break;
            case 2:
                current_state = State::Game;
                break;
        }

        if (get_inputs()->is_any_key_triggered())
            ++start_status;
    }

    void end()
    {
        draw(*the_end);
    }

    void on_init() override
    {
        logo = std::make_unique<Aporia::Sprite>(get_textures()->get_texture("logo"), sf::Vector2f(0, 0));
        press = std::make_unique<Aporia::Sprite>(get_textures()->get_texture("press"), sf::Vector2f(0, 0));
        rules = std::make_unique<Aporia::Sprite>(get_textures()->get_texture("rules"), sf::Vector2f(0, 0));
        the_end = std::make_unique<Aporia::Sprite>(get_textures()->get_texture("the_end"), sf::Vector2f(0, 0));

        levels = load_levels_from_file("game.json");
        load_sound("move.wav");
        load_sound("push.wav");
        load_sound("win.wav");
        start_level(lvl);
    }

    void on_update() override
    {
        switch (current_state)
        {
            case State::Start:
                start();
                break;
            case State::Game:
                draw_level();
                player = current_level.get_player();
                handle_inputs();
                break;
            case State::End:
                end();
                break;
        }
    }

    void on_terminate() override
    {
    }
};

std::unique_ptr<Aporia::Game> create_game()
{
    return std::make_unique<Example>();
}
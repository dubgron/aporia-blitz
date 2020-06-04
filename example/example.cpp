#include <aporia.hpp>

#include <iomanip>

#include <imgui.h>
#include <SFML/Audio.hpp>

#include "level_manager.hpp"

template<typename T>
int sign(T val)
{
    if (val > 0) return 1;
    if (val == 0) return 0;
    else return -1;
}

class Example : public Aporia::Game
{
public:
    using Game::Game;
    ~Example() {};

    void on_init() override
    {
        using json = nlohmann::json;

        std::string data = Aporia::read_file("game.json");
        json game = json::parse(data);

        player_speed = game["player_speed"];
        animal_speed = game["animal_speed"];

        player_pull = game["player_pull"];
        food_pull = game["food_pull"];

        volume = game["volume"];

        player = new Aporia::Sprite(_textures.get_texture("player"), sf::Vector2f(0, 0), Aporia::Origin::Bottom);
        animal = new Aporia::Sprite(_textures.get_texture("animal"), sf::Vector2f(0, 0), Aporia::Origin::Bottom);

        win = new Aporia::Sprite(_textures.get_texture("win"), sf::Vector2f(0, 0));
        tutorial = new Aporia::Sprite(_textures.get_texture("menu"), sf::Vector2f(0, 0));
        player_died = new Aporia::Sprite(_textures.get_texture("player_died"), sf::Vector2f(0, 0));
        animal_died = new Aporia::Sprite(_textures.get_texture("animal_died"), sf::Vector2f(0, 0));

        music.openFromFile("all_pigs_go_to_heaven.wav");
        music.setLoop(true);
        music.setVolume(volume);
        music.play();

        load();
    }

    void on_update(sf::Time delta_time) override
    {
        if (!player_dead && !animal_dead)
        {
            if (!win_screen && !tut_screen)
            {
                draw_level();
                move_player(delta_time);
                move_animal(delta_time);
                _camera.follow(player->get_position(), delta_time.asSeconds());
            }
            else
            {
                if (win_screen)
                    _renderer.draw(*win);
                else if (tut_screen)
                {
                    _renderer.draw(*tutorial);

                    if (_inputs.is_any_key_triggered())
                    {
                        tut_screen = false;
                        draw_level();
                        start_level(lvl);
                    }
                }

                _camera.get_camera().set_position(sf::Vector2f(320, 180));
            }

#           if defined(_DEBUG)
            debug();
#           endif
        }
        else
        {
            if (player_dead)
                _renderer.draw(*player_died);
            else if (animal_dead)
                _renderer.draw(*animal_died);

            _camera.get_camera().set_position(sf::Vector2f(320, 180));

            if (_inputs.is_any_key_triggered())
            {
                player_dead = animal_dead = false;
                draw_level();
                start_level(lvl);
            }
        }

    }

    void on_terminate() override
    {
        delete player;
        delete animal;
    }

    void load()
    {
        levels = load_levels_from_file("game.json");
    }

    void start_level(size_t level)
    {
        current_level = levels.get_level(level);

        auto player_pos = current_level.get_player();
        text_2f("Player_pos", player_pos.x, player_pos.y);
        player->set_position(player_pos.x * 32.0f, (current_level.height - player_pos.y) * 32.0f);

        auto animal_pos = current_level.get_animal();
        animal->set_position(animal_pos.x * 32.0f, (current_level.height - animal_pos.y) * 32.0f);

        _camera.get_camera().set_position(player->get_position());
    }

    void draw_level()
    {
        sf::Vector2f player_pos = player->get_position() + sf::Vector2f(16, 16);
        size_t pos_x = player_pos.x / 32;
        size_t pos_y = current_level.height - (player_pos.y) / 32 + 1;

        for (int y = 0; y < current_level.height; y++)
        {
            for (int x = 0; x < current_level.width; x++)
            {
                char c = current_level.get_cell(x, y);
                std::string tex = "";
                if (c == current_level.components.trap)
                    tex = "trap";
                else if (c == current_level.components.food)
                    tex = "food";
                else if (c == current_level.components.wall)
                    tex = "wall";

                Aporia::Sprite floor(_textures.get_texture("floor"), sf::Vector2f(x, current_level.height - y) * 32.0f);
                _renderer.draw(floor);

                if (tex != "")
                {
                    Aporia::Sprite to_draw(_textures.get_texture(tex), sf::Vector2f(x, current_level.height - y) * 32.0f, Aporia::Origin::Bottom);
                    _renderer.draw(to_draw, true);
                }
            }
        }

        _renderer.draw(*animal, true);
        _renderer.draw(*player, true);
    }

    void move_player(sf::Time delta_time)
    {
        float new_move_x = 0.0f;
        float new_move_y = 0.0f;

        sf::Vector2f player_pos = player->get_position() + sf::Vector2f(16, 16);

        if (_inputs.is_key_pressed(Aporia::Keyboard::W) || _inputs.is_key_pressed(Aporia::Keyboard::Up))
            new_move_y += player_speed;
        if (_inputs.is_key_pressed(Aporia::Keyboard::S) || _inputs.is_key_pressed(Aporia::Keyboard::Down))
            new_move_y -= player_speed;
        if (_inputs.is_key_pressed(Aporia::Keyboard::A) || _inputs.is_key_pressed(Aporia::Keyboard::Left))
            new_move_x -= player_speed;
        if (_inputs.is_key_pressed(Aporia::Keyboard::D) || _inputs.is_key_pressed(Aporia::Keyboard::Right))
            new_move_x += player_speed;

        float move_len = std::sqrt(new_move_x * new_move_x + new_move_y * new_move_y);
        if (move_len)
        {
            new_move_x = new_move_x / move_len * std::sqrt(player_speed * delta_time.asSeconds());
            new_move_y = new_move_y / move_len * std::sqrt(player_speed * delta_time.asSeconds());
        }

        if (new_move_x < 0)
            player->set_flip_x(true);
        else if (new_move_x > 0)
            player->set_flip_x(false);

        size_t x = player_pos.x / 32;
        size_t y = current_level.height - (player_pos.y) / 32 + 1;

        size_t new_x = (player_pos.x + new_move_x) / 32;
        size_t new_y = current_level.height - (player_pos.y + new_move_y) / 32 + 1;

        if (current_level.get_cell(new_x, y) == current_level.components.wall)
            new_move_x = 0.0f;
        if (current_level.get_cell(x, new_y) == current_level.components.wall)
            new_move_y = 0.0f;

        sf::Vector2f new_move = sf::Vector2f(new_move_x, new_move_y);
        player->move(new_move);

        player_dead = (current_level.get_cell(new_x, new_y) == current_level.components.trap);
    }

    void move_animal(sf::Time delta_time)
    {
        sf::Vector2f animal_pos = animal->get_position() + sf::Vector2f(16, 16);

        auto food_pos = current_level.get_food();
        sf::Vector2f food_vec = sf::Vector2f(food_pos.x * 32.0f, (current_level.height - food_pos.y) * 32.0f) - animal_pos;
        float food_len = std::sqrt(food_vec.x * food_vec.x + food_vec.y * food_vec.y);

        sf::Vector2f player_vec = player->get_position() - animal_pos;
        float player_len = std::sqrt(player_vec.x * player_vec.x + player_vec.y * player_vec.y);

        sf::Vector2f food_res;
        if (food_len)
            food_res = food_vec / food_len / food_len * food_pull;

        sf::Vector2f player_res;
        if (player_len)
            player_res = player_vec / player_len / player_len * player_pull;

        sf::Vector2f res = food_res + player_res;
        float res_len = std::sqrt(res.x * res.x + res.y * res.y);

        if (res_len)
            res = res / res_len * animal_speed * delta_time.asSeconds();

        if (res.x < 0)
            animal->set_flip_x(true);
        else if(res.x > 0)
            animal->set_flip_x(false);

        size_t x = animal_pos.x / 32;
        size_t y = current_level.height - animal_pos.y / 32 + 1;

        size_t new_x = (animal_pos.x + res.x) / 32;
        size_t new_y = current_level.height - (animal_pos.y + res.y) / 32 + 1;

        char c = current_level.get_cell(x, y);

        if (current_level.get_cell(new_x, y) == current_level.components.wall)
            res.x = 0.0f;
        if (current_level.get_cell(x, new_y) == current_level.components.wall)
            res.y = 0.0f;

        animal->move(res);

        animal_dead = (c == current_level.components.trap);

        if (c == current_level.components.food)
        {
            lvl += 1;
            if (lvl == levels.count())
                win_screen = true;
            else
                start_level(lvl);
        }
    }

    void debug()
    {
        float_slider("volume", &volume, 0, 100);
        music.setVolume(volume);

        float_slider("player_speed", &player_speed, 20.0f, 500.0f);
        float_slider("animal_speed", &animal_speed, 20.0f, 500.0f);
        float_slider("player_pull", &player_pull, -100.0f, 100.0f);
        float_slider("food_pull", &food_pull, -100.0f, 100.0f);
        
        if (button("reload"))
        {
            load();
            start_level(lvl);
        }
        if (button("restart"))
        {
            lvl = 0;
            start_level(lvl);
        }
        if (button("save sliders"))
        {
            using json = nlohmann::json;

            std::string data = Aporia::read_file("game.json");
            json game = json::parse(data);

            game["player_speed"] = player_speed;
            game["animal_speed"] = animal_speed;

            game["player_pull"] = player_pull;
            game["food_pull"] = food_pull;

            game["volume"] = volume;

            std::ofstream file("game.json");
            file << std::setw(4) << game;
        }

        int level = lvl;
        int_slider("Level", &level, 0, levels.count() - 1);
        if (level != lvl)
        {
            lvl = level;
            start_level(lvl);
        }
    }

    sf::Music music;
    float volume;

    Aporia::Sprite* player = nullptr;
    Aporia::Sprite* animal = nullptr;

    float player_speed = 100.0f;
    float animal_speed = 50.0f;

    float player_pull = -10.0f;
    float food_pull = 20.0f;

    bool player_dead = false;
    bool animal_dead = false;

    LevelManager levels;
    size_t lvl = 0;
    Level current_level;

    bool win_screen = false;
    bool tut_screen = true;

    Aporia::Sprite* tutorial;
    Aporia::Sprite* win;
    Aporia::Sprite* player_died;
    Aporia::Sprite* animal_died;
};

std::unique_ptr<Aporia::Game> create_game()
{
    return std::make_unique<Example>("config.json");
}

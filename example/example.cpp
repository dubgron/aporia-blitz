#include <aporia.hpp>

#include "credits_scene.hpp"
#include "end_scene.hpp"
#include "game_scene.hpp"
#include "menu_scene.hpp"

class Example : public Aporia::Game
{
public:
    using Game::Game;
    ~Example() {};

    void center_sprite(Aporia::Sprite& s)
    {
        auto& transform = s.get_component<Aporia::Transform2D>();

        transform.position -= glm::vec3{ transform.origin, 0.0f };
    }

    void on_init() override
    {
        auto exit = std::bind(&Example::load_scene, this, std::placeholders::_1);

        Aporia::Sprite menu_screen(_textures.get_texture("menu"), glm::vec3{ 0.0f, 0.0f, -0.9f }, glm::vec2{ 800.0f, 800.0f });
        center_sprite(menu_screen);
        menu = std::make_shared<MenuScene>(_logger, exit, menu_screen);

        Aporia::Sprite heart(_textures.get_texture("heart"), glm::vec3{ 0.0f, 0.0f, 0.3f }, glm::vec2{ 32.0f });
        center_sprite(heart);
        game = std::make_shared<GameScene>(_logger, exit, _camera, heart);

        Aporia::Sprite credits_sprite(_textures.get_texture("credits"), glm::vec3{ 0.0f }, glm::vec2{ 800.0f });
        center_sprite(credits_sprite);
        credits = std::make_shared<CreditsScene>(_logger, exit, std::move(credits_sprite));

        Aporia::Sprite end_sprite(_textures.get_texture("end"), glm::vec3{ 0.0f }, glm::vec2{ 800.0f });
        center_sprite(end_sprite);
        end = std::make_shared<EndScene>(_logger, exit, std::move(end_sprite));

        _scenes.add(menu);
        _scenes.add(game);
        _scenes.add(credits);
        _scenes.add(end);

        _scenes.load_scene(menu);

    }

    void on_update(Aporia::Deltatime dt) override
    {
        //if (_inputs.is_key_triggered(Aporia::Keyboard::Num1))
        //    load_scene(-1);
        //else if (_inputs.is_key_triggered(Aporia::Keyboard::Num2))
        //    load_scene(0);
        //else if (_inputs.is_key_triggered(Aporia::Keyboard::Num3))
        //    load_scene(1);
        //else if (_inputs.is_key_triggered(Aporia::Keyboard::Num4))
        //    load_scene(2);
        //else if (_inputs.is_key_triggered(Aporia::Keyboard::Num5))
        //    load_scene(3);
    }

    void on_terminate() override
    {
    }

    void load_scene(int scene_id)
    {
        switch (scene_id)
        {
            case -1: _scenes.load_scene(menu); break;
            case 0: _scenes.load_scene(game); break;
            case 1: _scenes.load_scene(credits); break;
            case 2: _window.close(); break;
            case 3: _scenes.load_scene(end); break;
        }
    }

    std::shared_ptr<MenuScene> menu;
    std::shared_ptr<GameScene> game;
    std::shared_ptr<CreditsScene> credits;
    std::shared_ptr<EndScene> end;

    std::shared_ptr<Aporia::Sprite> background;
};

std::unique_ptr<Aporia::Game> create_game()
{
    return std::make_unique<Example>("config.json");
}

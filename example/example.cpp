#include <aporia.hpp>

class Example : public Aporia::Game
{
public:
    using Game::Game;
    ~Example() {};

    void on_init() override
    {
    }

    void on_update(f32 time, f32 delta_time) override
    {
    }

    void on_draw() override
    {

    }

    void on_terminate() override
    {
    }
};

std::unique_ptr<Aporia::Game> create_game()
{
    return std::make_unique<Example>("assets/config.json");
}

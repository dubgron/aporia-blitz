#include <aporia.hpp>

class Example : public Aporia::Game
{
public:
    ~Example() {};

    void on_init() override
    {
    }

    void on_update() override
    {
    }

    void on_terminate() override
    {
    }
};

std::unique_ptr<Aporia::Game> create_game()
{
    return std::make_unique<Example>();
}
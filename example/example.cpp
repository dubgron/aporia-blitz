#include <aporia.hpp>

void game_init()
{

}

void game_update(f32 time, f32 delta_time)
{

}

void game_draw(f32 frame_time)
{

}

void game_terminate()
{

}

Aporia::Game create_game()
{
    Aporia::Game game;

    game.init               = game_init;
    game.update             = game_update;
    game.draw               = game_draw;
    game.terminate          = game_terminate;
    game.config_filepath    = "assets/config.json";

    return game;
}

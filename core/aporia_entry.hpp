#pragma once

#include "aporia_game.hpp"

extern Aporia::Game create_game();

int main(int argc, char** argv)
{
    Aporia::Game game = create_game();
    Aporia::game_run(&game);

    return 0;
}

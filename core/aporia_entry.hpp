#pragma once

#include "aporia_debug.hpp"
#include "aporia_game.hpp"

extern std::unique_ptr<Aporia::Game> create_game();

int main(int argc, char** argv)
{
    Aporia::logging_init("aporia");

    const std::unique_ptr<Aporia::Game> game = create_game();
    game->run();

    return 0;
}

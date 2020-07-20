#pragma once

#include "game.hpp"

extern std::unique_ptr<Aporia::Game> create_game();

int main(int argc, char** argv)
{
    auto game = create_game();
    game->run();

    return 1;
}

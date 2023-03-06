#pragma once

#include "game.hpp"

extern std::unique_ptr<Aporia::Game> create_game();

int main(int argc, char** argv)
{
    Aporia::Logger::Init("aporia");

    const std::unique_ptr<Aporia::Game> game = create_game();
    game->run();

    return 0;
}

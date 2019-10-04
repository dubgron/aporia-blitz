#pragma once

#include "engine.hpp"

extern std::unique_ptr<Aporia::Game> create_game();

int main(int argc, char** argv)
{
    Aporia::Engine engine("config.json");
    engine.run(create_game());

    return 1;
}

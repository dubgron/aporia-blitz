#pragma once

#include "aporia_memory.hpp"
#include "aporia_world.hpp"

namespace Aporia
{
    struct GameMemory
    {
        MemoryArena persistent;
        MemoryArena frame;
        MemoryArena temp[2];
        MemoryArena config;
    };

    extern GameMemory memory;
    extern World world;

    void engine_main(String config_filepath);
}

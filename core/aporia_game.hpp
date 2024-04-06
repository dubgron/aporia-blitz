#pragma once

#include "aporia_memory.hpp"
#include "aporia_world.hpp"

struct GameMemory
{
    MemoryArena persistent;
    MemoryArena frame;
    MemoryArena config;
    MemoryArena assets;
};

extern GameMemory memory;
extern World world;

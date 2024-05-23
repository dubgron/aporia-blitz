#pragma once

#include "aporia_memory.hpp"

struct GameMemory
{
    MemoryArena persistent;
    MemoryArena frame;
    MemoryArena config;
    MemoryArena assets;
};

extern GameMemory memory;

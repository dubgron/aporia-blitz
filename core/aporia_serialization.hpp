#pragma once

#include "aporia_entity.hpp"
#include "aporia_string.hpp"
#include "aporia_types.hpp"
#include "aporia_world.hpp"

struct Serializer
{
    String buffer;
    u64 offset = 0;

    // Idea: have a separate data segment to store arrays

    MemoryArena* context_arena = nullptr;
};

String world_serialize(MemoryArena* arena, const World& world);
World world_deserialize(String serialized);

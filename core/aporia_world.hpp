#pragma once

#include "aporia_entity.hpp"
#include "aporia_memory.hpp"

struct World
{
    MemoryArena arena;

    Entity* entity_array = nullptr;
    i32 entity_max_count = 0;
    i32 entity_count = 0;

    Entity* free_list = nullptr;
};

World world_init(i32 max_entities = 10000);
void world_deinit(World* world);

EntityID entity_create(World* world, Entity** out_entity = nullptr);
void entity_destroy(World* world, EntityID entity_id);
Entity* entity_get(World* world, EntityID entity_id);

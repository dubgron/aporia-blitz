#pragma once

#include "aporia_entity.hpp"
#include "aporia_memory.hpp"

struct EntityID
{
    i32 index = INDEX_INVALID;
    i32 generation = INDEX_INVALID;
};

struct EntityNode
{
    i32 generation = INDEX_INVALID;

    EntityNode* next = nullptr;
    Entity* entity = nullptr;
};

struct World
{
    MemoryArena arena;
    i64 max_entities = 0;

    Entity* entity_array = nullptr;
    i64 entity_count = 0;

    EntityNode* entity_list = nullptr;
    EntityNode* free_list = nullptr;
};

World world_init(i64 in_max_entities = 10000);
void world_deinit(World* world);

void world_clear(World* world);

EntityID entity_create(World* world, Entity** out_entity = nullptr);
void entity_destroy(World* world, EntityID entity_id);
Entity* entity_get(World* world, EntityID entity_id);

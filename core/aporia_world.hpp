#pragma once

#include "aporia_entity.hpp"
#include "aporia_memory.hpp"

namespace Aporia
{
    struct EntityID
    {
        u64 index = 0;
        u64 generation = 0;
    };

    struct EntityNode
    {
        u64 generation = 0;

        EntityNode* next = nullptr;
        Entity* entity = nullptr;
    };

    struct World
    {
        MemoryArena arena;
        u64 max_entities = 0;

        Entity* entity_array = nullptr;
        u64 entity_count = 0;

        EntityNode* entity_list = nullptr;
        EntityNode* free_list = nullptr;
    };

    World world_init(u64 in_max_entities = 10000);
    void world_deinit(World* world);

    void world_clear(World* world);

    String world_serialize(MemoryArena* arena, World* world);
    World world_deserialize(String serialized);

    EntityID entity_create(World* world, Entity** out_entity = nullptr);
    void entity_destroy(World* world, EntityID entity_id);
    Entity* entity_get(World* world, EntityID entity_id);
}

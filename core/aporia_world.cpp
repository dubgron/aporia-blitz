#include "aporia_world.hpp"

#include "aporia_debug.hpp"

World current_world;

World world_init(i32 max_entities /* = 10000 */)
{
    World world;
    world.entity_max_count = max_entities;

    // @TODO(dubgron): The count of the world arena should be more planned out.
    i64 world_arena_size = world.entity_max_count * sizeof(Entity) * 4;
    world.arena = arena_init(world_arena_size);
    APORIA_LOG(Info, "World has allocated % B of memory.", world_arena_size);

    world.entity_array = arena_push<Entity>(&world.arena, world.entity_max_count);
    world.entity_array_last_frame = arena_push<Entity>(&world.arena, world.entity_max_count);

    return world;
}

void world_deinit(World* world)
{
    arena_deinit(&world->arena);
}

void world_next_frame(World* world)
{
    for (i64 idx = 0; idx < world->entity_count; ++idx)
    {
        entity_flags_unset(&world->entity_array[idx], EntityFlag_SkipFrameInterpolation);
    }

    memcpy(world->entity_array_last_frame, world->entity_array, world->entity_count * sizeof(Entity));
}

EntityID entity_create(World* world, Entity** out_entity)
{
    Entity* entity = nullptr;

    if (world->free_list)
    {
        entity = world->free_list;
        world->free_list = world->free_list->next;
    }
    else
    {
        APORIA_ASSERT_WITH_MESSAGE(world->entity_count < world->entity_max_count,
            "The entity array is full! Can't create a new Entity!");

        entity = &world->entity_array[world->entity_count];
        world->entity_count += 1;

        entity->id.index = INDEX_IN_ARRAY(entity, world->entity_array);
        entity->id.generation = 0;
    }

    entity_flags_set(entity, EntityFlag_Active);

    if (out_entity)
    {
        *out_entity = entity;
    }

    return entity->id;
}

void entity_destroy(World* world, EntityID entity_id)
{
    i32 index = entity_id.index;
    i32 generation = entity_id.generation;

    APORIA_ASSERT_WITH_MESSAGE(index >= 0 && index < world->entity_max_count && generation >= 0,
        "Invalid Entity ID (index: %, generation: %)!", index, generation);

    Entity* entity = &world->entity_array[index];

    APORIA_ASSERT_WITH_MESSAGE(entity->id.generation == generation,
        "Generation mismatch! Tried to remove Entity with ID (index: %, generation: %), but only found ID (index: %, generation: %)!",
        index, generation, index, entity->id.generation);

    APORIA_ASSERT_WITH_MESSAGE(entity_flags_has_all(*entity, EntityFlag_Active),
        "Entity with ID (index: %, generation: %) is not alive!", index, generation);

    entity_flags_unset(entity, EntityFlag_Active);

    entity->id.generation += 1;

    entity->next = world->free_list;
    world->free_list = entity;
}

Entity* entity_get(World* world, EntityID entity_id)
{
    i32 index = entity_id.index;
    i32 generation = entity_id.generation;

    APORIA_ASSERT_WITH_MESSAGE(index >= 0 && index < world->entity_max_count && generation >= 0,
        "Invalid EntityID (index: %, generation: %)!", index, generation);

    Entity* entity = &world->entity_array[index];
    if (entity->id.generation != generation)
    {
        return nullptr;
    }

    return entity;
}

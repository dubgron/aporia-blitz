#include "aporia_world.hpp"

#include "aporia_debug.hpp"

#define INDEX_IN_ARRAY(elem, array) ((PTR_TO_INT(elem) - PTR_TO_INT(array)) / sizeof(*elem))

namespace Aporia
{
    void world_init(World* world, u64 in_max_entities /* = 10000 */)
    {
        world->max_entities = in_max_entities;
        world->entity_count = 0;

        // @TODO(dubgron): The count of the world arena should be more planned out.
        const u64 world_arena_size = 2 * world->max_entities * (sizeof(Entity) + sizeof(EntityNode));
        world->arena.alloc(world_arena_size);
        APORIA_LOG(Info, "World has allocated % B of memory.", world_arena_size);

        world->entity_array = world->arena.push_zero<Entity>(world->max_entities);
        world->entity_list = world->arena.push_zero<EntityNode>(world->max_entities);

        clear_entities(world);
    }

    void world_deinit(World* world)
    {
        world->arena.dealloc();
    }

    EntityID create_entity(World* world, Entity** out_entity)
    {
        APORIA_ASSERT_WITH_MESSAGE(world->free_list,
            "The free list is empty! Can't create new Entity");

        // Pop an element from the free list.
        EntityNode* free_node = world->free_list;
        world->free_list = world->free_list->next;

        // Get a new entity from the end of the array.
        Entity* new_entity = &world->entity_array[world->entity_count];
        world->entity_count += 1;

        // Clean up the new entity and assign the index of the node.
        *new_entity = Entity{};
        new_entity->index = INDEX_IN_ARRAY(free_node, world->entity_list);

        // Assign the new entity to the node and increment the generation.
        free_node->entity = new_entity;
        free_node->generation += 1;

        // Assign the output entity, if required.
        if (out_entity)
        {
            *out_entity = new_entity;
        }

        return EntityID{ new_entity->index, free_node->generation };
    }

    void remove_entity(World* world, EntityID entity_id)
    {
        const u64 index = entity_id.index;
        const u64 generation = entity_id.generation;

        APORIA_ASSERT_WITH_MESSAGE(index < world->max_entities && generation > 0,
            "Invalid Entity ID (index: %, generation: %)!", index, generation);

        EntityNode* entity_node = &world->entity_list[index];

        APORIA_ASSERT_WITH_MESSAGE(entity_node->generation == generation,
            "Generation mismatch! Tried to remove Entity with ID (index: %, generation: %), but only found ID (index: %, generation: %)!",
            index, generation, index, entity_node->generation);

        APORIA_ASSERT_WITH_MESSAGE(entity_node->entity != nullptr,
            "Entity with ID (index: %, generation: %) is null!", index, generation);

        const Entity* last_entity = &world->entity_array[world->entity_count - 1];

        // Swap the removed entity with the end of the entity array and redirect its node.
        *entity_node->entity = *last_entity;
        world->entity_list[last_entity->index].entity = entity_node->entity;
        world->entity_count -= 1;

        // Clear the node of the removed entity and push it onto the free list.
        entity_node->entity = nullptr;
        entity_node->next = world->free_list;
        world->free_list = entity_node;
    }

    void clear_entities(World* world)
    {
        world->entity_count = 0;
        world->free_list = world->entity_list;

        const u64 last_idx = world->max_entities - 1;
        for (u64 idx = 0; idx < last_idx; ++idx)
        {
            world->entity_list[idx].next = &world->entity_list[idx + 1];
            world->entity_list[idx].entity = nullptr;
        }

        world->entity_list[last_idx].next = nullptr;
        world->entity_list[last_idx].entity = nullptr;
    }

    Entity* get_entity(World* world, EntityID entity_id)
    {
        const u64 index = entity_id.index;
        const u64 generation = entity_id.generation;

        APORIA_ASSERT_WITH_MESSAGE(index < world->max_entities && generation > 0,
            "Invalid EntityID (index: %, generation: %)!", index, generation);

        const EntityNode* entity_node = &world->entity_list[index];
        if (entity_node->generation != generation)
        {
            return nullptr;
        }

        return entity_node->entity;
    }
}

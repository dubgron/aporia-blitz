#include "aporia_world.hpp"

#include "aporia_debug.hpp"

#define INDEX_IN_ARRAY(elem, array) ((PTR_TO_INT(elem) - PTR_TO_INT(array)) / sizeof(*elem))

namespace Aporia
{
    void World::init(u64 in_max_entities /* = 10000 */)
    {
        max_entities = in_max_entities;

        // @TODO(dubgron): The size of the world arena should be more planned out.
        const u64 world_arena_size = 2 * max_entities * (sizeof(Entity) + sizeof(EntityNode));
        world_arena.alloc(world_arena_size);
        APORIA_LOG(Info, "World has allocated {} B of memory.", world_arena_size);

        entity_array = world_arena.push_zero<Entity>(max_entities);
        entity_list = world_arena.push_zero<EntityNode>(max_entities);

        clear_entities();
    }

    void World::deinit()
    {
        world_arena.dealloc();
    }

    EntityID World::create_entity(Entity** out_entity)
    {
        APORIA_ASSERT_WITH_MESSAGE(free_list,
            "The free list is empty! Can't create new Entity");

        // Pop an element from the free list.
        EntityNode* free_node = free_list;
        free_list = free_list->next;

        // Get a new entity from the end of the array.
        Entity* new_entity = &entity_array[entity_count];
        entity_count += 1;

        // Clean up the new entity and assign the index of the node.
        *new_entity = Entity{};
        // @TOOD(dubgron): I don't like it. It should be handled differently in the future.
        new_entity->animator.owner = new_entity;
        new_entity->index = INDEX_IN_ARRAY(free_node, entity_list);

        // Assign the new entity to the node and increment the generation.
        free_node->entity = new_entity;
        free_node->generation += 1;

        // Assign the output entity, if required.
        if (out_entity)
        {
            *out_entity = free_node->entity;
        }

        return EntityID{ new_entity->index, free_node->generation };
    }

    void World::remove_entity(EntityID entity_id)
    {
        const u64 index = entity_id.index;
        const u64 generation = entity_id.generation;

        APORIA_ASSERT_WITH_MESSAGE(index < max_entities && generation > 0,
            "Invalid Entity ID (index: {}, generation: {})!", index, generation);

        EntityNode* entity_node = &entity_list[index];

        APORIA_ASSERT_WITH_MESSAGE(entity_node->generation == generation,
            "Generation mismatch! Tried to remove Entity with ID (index: {}, generation: {}), but only found ID (index: {}, generation: {})!",
            index, generation, index, entity_node->generation);

        APORIA_ASSERT_WITH_MESSAGE(entity_node->entity != nullptr,
            "Entity with ID (index: {}, generation: {}) is null!", index, generation);

        // Swap the removed entity with the end of the entity array and redirect its node.
        const Entity* last_entity = &entity_array[entity_count - 1];
        entity_count -= 1;
        *entity_node->entity = *last_entity;
        entity_list[last_entity->index].entity = entity_node->entity;

        // Clear the node of the removed entity and push it onto the free list.
        entity_node->entity = nullptr;
        entity_node->next = free_list;
        free_list = entity_node;
}

    void World::clear_entities()
    {
        entity_count = 0;

        free_list = entity_list;
        for (u64 idx = 0; idx < max_entities - 1; ++idx)
        {
            entity_list[idx].next = &entity_list[idx + 1];
            entity_list[idx].entity = nullptr;
        }
    }

    Entity* World::get_entity(EntityID entity_id) const
    {
        const u64 index = entity_id.index;
        const u64 generation = entity_id.generation;

        APORIA_ASSERT_WITH_MESSAGE(index < max_entities && generation > 0,
            "Invalid EntityID (index: {}, generation: {})!", index, generation);

        const EntityNode* entity_node = &entity_list[index];
        if (entity_node->generation != generation)
        {
            return nullptr;
        }

        return entity_node->entity;
    }
}

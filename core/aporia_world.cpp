#include "aporia_world.hpp"

#include "aporia_debug.hpp"

namespace Aporia
{
    World world_init(u64 in_max_entities /* = 10000 */)
    {
        World result;
        result.max_entities = in_max_entities;
        result.entity_count = 0;

        // @TODO(dubgron): The count of the world arena should be more planned out.
        u64 world_arena_size = 2 * result.max_entities * (sizeof(Entity) + sizeof(EntityNode));
        result.arena = arena_init(world_arena_size);
        APORIA_LOG(Info, "World has allocated % B of memory.", world_arena_size);

        result.entity_array = arena_push<Entity>(&result.arena, result.max_entities);
        result.entity_list = arena_push<EntityNode>(&result.arena, result.max_entities);

        world_clear(&result);

        return result;
    }

    void world_deinit(World* world)
    {
        arena_deinit(&world->arena);
    }

    void world_clear(World* world)
    {
        world->entity_count = 0;
        world->free_list = world->entity_list;

        u64 last_idx = world->max_entities - 1;
        for (u64 idx = 0; idx < last_idx; ++idx)
        {
            world->entity_list[idx].next = &world->entity_list[idx + 1];
            world->entity_list[idx].entity = nullptr;
        }

        world->entity_list[last_idx].next = nullptr;
        world->entity_list[last_idx].entity = nullptr;
    }

    struct Serializer
    {
        String buffer;
        u64 offset = 0;
    };

    template<typename T>
    void serialize_write_array(Serializer* serializer, T* value, u64 count)
    {
        u64 size = sizeof(T) * count;
        memcpy(serializer->buffer.data + serializer->offset, value, size);
        serializer->offset += size;
    }

    template<typename T>
    void serialize_write_value(Serializer* serializer, T value)
    {
        serialize_write_array(serializer, &value, 1);
    }

    template<typename T>
    T* serialize_read_array(Serializer* serializer, u64 count)
    {
        T* data = (T*)(serializer->buffer.data + serializer->offset);

        u64 size = sizeof(T) * count;
        serializer->offset += size;

        return data;
    }

    template<typename T>
    T serialize_read_value(Serializer* serializer)
    {
        return *serialize_read_array<T>(serializer, 1);
    }

    String world_serialize(MemoryArena* arena, World* world)
    {
        u64 relevant_list_items = 0;
        for (i64 idx = world->max_entities - 1; idx >= 0; --idx)
        {
            if (world->entity_list[idx].generation > 0)
            {
                relevant_list_items = idx + 1;
                break;
            }
        }

        u64 needed_size = 2 * sizeof(u64)           // max_entities and entity_count
            + world->entity_count * sizeof(Entity)  // entity_array
            + 1 * sizeof(u64)                       // relevant_list_items
            + relevant_list_items * sizeof(u64);    // generational indices

        Serializer serializer;
        serializer.buffer = push_string(arena, needed_size);

        serialize_write_value(&serializer, world->max_entities);
        serialize_write_value(&serializer, world->entity_count);

        serialize_write_array(&serializer, world->entity_array, world->entity_count);

        serialize_write_value(&serializer, relevant_list_items);
        u64* generational_indices = serialize_read_array<u64>(&serializer, relevant_list_items);

        for (u64 idx = 0; idx < relevant_list_items; ++idx)
        {
            generational_indices[idx] = world->entity_list[idx].generation;
        }

        return serializer.buffer;
    }

    World world_deserialize(String serialized)
    {
        Serializer serializer;
        serializer.buffer = serialized;

        u64 max_entities = serialize_read_value<u64>(&serializer);
        World result = world_init(max_entities);

        result.entity_count = serialize_read_value<u64>(&serializer);
        Entity* entity_array = serialize_read_array<Entity>(&serializer, result.entity_count);

        for (u64 idx = 0; idx < result.entity_count; ++idx)
        {
            result.entity_array[idx] = entity_array[idx];

            Entity* entity = &result.entity_array[idx];
            result.entity_list[entity->index].entity = entity;
        }

        u64 relevant_list_items = serialize_read_value<u64>(&serializer);
        u64* generational_indices = serialize_read_array<u64>(&serializer, relevant_list_items);

        for (u64 idx = 0; idx < relevant_list_items; ++idx)
        {
            result.entity_list[idx].generation = generational_indices[idx];
        }

        // Reconstruct the free_list, starting from the end of the entity_list.
        result.free_list = nullptr;
        for (i64 idx = result.max_entities - 1; idx >= 0; --idx)
        {
            if (result.entity_list[idx].entity == nullptr)
            {
                result.entity_list[idx].next = result.free_list;
                result.free_list = &result.entity_list[idx];
            }
        }

        return result;
    }

    EntityID entity_create(World* world, Entity** out_entity)
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

    void entity_destroy(World* world, EntityID entity_id)
    {
        u64 index = entity_id.index;
        u64 generation = entity_id.generation;

        APORIA_ASSERT_WITH_MESSAGE(index < world->max_entities && generation > 0,
            "Invalid Entity ID (index: %, generation: %)!", index, generation);

        EntityNode* entity_node = &world->entity_list[index];

        APORIA_ASSERT_WITH_MESSAGE(entity_node->generation == generation,
            "Generation mismatch! Tried to remove Entity with ID (index: %, generation: %), but only found ID (index: %, generation: %)!",
            index, generation, index, entity_node->generation);

        APORIA_ASSERT_WITH_MESSAGE(entity_node->entity != nullptr,
            "Entity with ID (index: %, generation: %) is null!", index, generation);

        Entity* last_entity = &world->entity_array[world->entity_count - 1];

        // Swap the removed entity with the end of the entity array and redirect its node.
        *entity_node->entity = *last_entity;
        world->entity_list[last_entity->index].entity = entity_node->entity;
        world->entity_count -= 1;

        // Clear the node of the removed entity and push it onto the free list.
        entity_node->entity = nullptr;
        entity_node->next = world->free_list;
        world->free_list = entity_node;
    }

    Entity* entity_get(World* world, EntityID entity_id)
    {
        u64 index = entity_id.index;
        u64 generation = entity_id.generation;

        APORIA_ASSERT_WITH_MESSAGE(index < world->max_entities && generation > 0,
            "Invalid EntityID (index: %, generation: %)!", index, generation);

        EntityNode* entity_node = &world->entity_list[index];
        if (entity_node->generation != generation)
        {
            return nullptr;
        }

        return entity_node->entity;
    }
}

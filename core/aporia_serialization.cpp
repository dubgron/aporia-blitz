#include "aporia_serialization.hpp"

#define push_context_arena(serializer, arena) \
    for (MemoryArena *old = serializer->context_arena, *next = (arena), *pop = 0; \
        (serializer->context_arena = next, !pop); \
        next = old, pop = (MemoryArena*)1)

namespace Aporia
{
    template<typename T>
    void serialize_write(Serializer* serializer, T value)
    {
        u64 size = sizeof(T);
        void* data = serializer->buffer.data + serializer->offset;
        serializer->offset += size;

        *(T*)data = value;
    }

    template<typename T>
    void serialize_write_array(Serializer* serializer, T* array, u64 count)
    {
        u64 size = sizeof(T) * count;
        void* data = serializer->buffer.data + serializer->offset;
        serializer->offset += size;

        memcpy(data, array, size);
    }

    template<typename T>
    void serialize_read(Serializer* serializer, T* out_value)
    {
        u64 size = sizeof(T);
        void* data = serializer->buffer.data + serializer->offset;
        serializer->offset += size;

        *out_value = *(T*)data;
    }

    template<typename T>
    void serialize_read_array(Serializer* serializer, T** out_array, u64 count)
    {
        u64 size = sizeof(T) * count;
        void* data = serializer->buffer.data + serializer->offset;
        serializer->offset += size;

        if (serializer->context_arena)
        {
            *out_array = arena_push_uninitialized<T>(serializer->context_arena, count);
            memcpy(*out_array, data, size);
        }
        else
        {
            *out_array = (T*)data;
        }
    }

    void serialize_write(Serializer* serializer, String string)
    {
        serialize_write(serializer, string.length);
        serialize_write_array(serializer, string.data, string.length);
    }
    void serialize_read(Serializer* serializer, String* string)
    {
        serialize_read(serializer, &string->length);
        serialize_read_array(serializer, &string->data, string->length);
    }

    void serialize_write(Serializer* serializer, const SubTexture& subtexture)
    {
        String subtexture_name;
        if (subtexture.texture_index != INDEX_INVALID)
        {
            subtexture_name = get_subtexture_name(subtexture);
        }
        serialize_write(serializer, subtexture_name);
    }

    void serialize_read(Serializer* serializer, SubTexture* subtexture)
    {
        String subtexture_name;
        serialize_read(serializer, &subtexture_name);
        if (subtexture_name.length > 0)
        {
            *subtexture = *get_subtexture(subtexture_name);
        }
    }

    void serialize_write(Serializer* serializer, const Animator& animator)
    {
        serialize_write(serializer, animator.current_animation);
        serialize_write(serializer, animator.requested_animation);

        serialize_write(serializer, animator.current_frame);
        serialize_write(serializer, animator.elapsed_time);
    }

    void serialize_read(Serializer* serializer, Animator* animator)
    {
        serialize_read(serializer, &animator->current_animation);
        serialize_read(serializer, &animator->requested_animation);

        serialize_read(serializer, &animator->current_frame);
        serialize_read(serializer, &animator->elapsed_time);
    }

    void serialize_write(Serializer* serializer, const Collider_Polygon& polygon)
    {
        serialize_write(serializer, polygon.point_count);
        serialize_write_array(serializer, polygon.points, polygon.point_count);
    }

    void serialize_read(Serializer* serializer, Collider_Polygon* polygon)
    {
        serialize_read(serializer, &polygon->point_count);
        serialize_read_array(serializer, &polygon->points, polygon->point_count);
    }

    void serialize_write(Serializer* serializer, const Collider& collider)
    {
        serialize_write(serializer, collider.type);
        switch (collider.type)
        {
            case ColliderType_AABB:     serialize_write(serializer, collider.aabb);     break;
            case ColliderType_Circle:   serialize_write(serializer, collider.circle);   break;
            case ColliderType_Polygon:  serialize_write(serializer, collider.polygon);  break;
        }
    }

    void serialize_read(Serializer* serializer, Collider* collider)
    {
        serialize_read(serializer, &collider->type);
        switch (collider->type)
        {
            case ColliderType_AABB:     serialize_read(serializer, &collider->aabb);     break;
            case ColliderType_Circle:   serialize_read(serializer, &collider->circle);   break;
            case ColliderType_Polygon:  serialize_read(serializer, &collider->polygon);  break;
        }
    }

    void entity_serialize(Serializer* serializer, const Entity& entity)
    {
        serialize_write(serializer, entity.index);

        serialize_write(serializer, entity.flags);

        serialize_write(serializer, entity.position);
        serialize_write(serializer, entity.z);

        serialize_write(serializer, entity.rotation);
        serialize_write(serializer, entity.center_of_rotation);

        serialize_write(serializer, entity.width);
        serialize_write(serializer, entity.height);
        serialize_write(serializer, entity.scale);

        serialize_write(serializer, entity.texture);
        serialize_write(serializer, entity.color);
        serialize_write(serializer, entity.shader_id);

        serialize_write(serializer, entity.animator);
        serialize_write(serializer, entity.collider);
    }

    void entity_deserialize(Serializer* serializer, Entity* entity, World* world)
    {
        serialize_read(serializer, &entity->index);

        serialize_read(serializer, &entity->flags);

        serialize_read(serializer, &entity->position);
        serialize_read(serializer, &entity->z);

        serialize_read(serializer, &entity->rotation);
        serialize_read(serializer, &entity->center_of_rotation);

        serialize_read(serializer, &entity->width);
        serialize_read(serializer, &entity->height);
        serialize_read(serializer, &entity->scale);

        serialize_read(serializer, &entity->texture);
        serialize_read(serializer, &entity->color);
        serialize_read(serializer, &entity->shader_id);

        serialize_read(serializer, &entity->animator);

        push_context_arena(serializer, &world->arena)
        {
            serialize_read(serializer, &entity->collider);
        }
    }

    String world_serialize(MemoryArena* arena, const World& world)
    {
        Serializer serializer;
        serializer.buffer = push_string(arena, MEGABYTES(1));

        serialize_write(&serializer, world.max_entities);
        serialize_write(&serializer, world.entity_count);

        for (i64 idx = 0; idx < world.entity_count; ++idx)
        {
            entity_serialize(&serializer, world.entity_array[idx]);
        }

        u64 relevant_list_items = 0;
        for (i64 idx = world.max_entities - 1; idx >= 0; --idx)
        {
            if (world.entity_list[idx].generation > 0)
            {
                relevant_list_items = idx + 1;
                break;
            }
        }
        serialize_write(&serializer, relevant_list_items);

        u64* generational_indices;
        serialize_read_array(&serializer, &generational_indices, relevant_list_items);

        for (u64 idx = 0; idx < relevant_list_items; ++idx)
        {
            generational_indices[idx] = world.entity_list[idx].generation;
        }

        u64 unused_memory = serializer.buffer.length - serializer.offset;
        arena_pop(arena, unused_memory);

        return String{ serializer.buffer.data, serializer.offset };
    }

    World world_deserialize(String serialized)
    {
        Serializer serializer;
        serializer.buffer = serialized;

        u64 max_entities;
        serialize_read(&serializer, &max_entities);
        World world = world_init(max_entities);

        serialize_read(&serializer, &world.entity_count);

        for (u64 idx = 0; idx < world.entity_count; ++idx)
        {
            Entity* entity = &world.entity_array[idx];
            entity_deserialize(&serializer, entity, &world);

            world.entity_list[entity->index].entity = entity;
        }

        u64 relevant_list_items;
        serialize_read(&serializer, &relevant_list_items);

        u64* generational_indices;
        serialize_read_array(&serializer, &generational_indices, relevant_list_items);

        for (u64 idx = 0; idx < relevant_list_items; ++idx)
        {
            world.entity_list[idx].generation = generational_indices[idx];
        }

        // Reconstruct the free_list, starting from the end of the entity_list.
        world.free_list = nullptr;
        for (i64 idx = world.max_entities - 1; idx >= 0; --idx)
        {
            if (world.entity_list[idx].entity == nullptr)
            {
                world.entity_list[idx].next = world.free_list;
                world.free_list = &world.entity_list[idx];
            }
        }

        return world;
    }
}

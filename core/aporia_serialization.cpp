#include "aporia_serialization.hpp"

#define push_context_arena(serializer, arena) \
    for (MemoryArena *old = serializer->context_arena, *next = (arena), *pop = 0; \
        (serializer->context_arena = next, !pop); \
        next = old, pop = (MemoryArena*)1)

template<typename T>
static void serialize_write(Serializer* serializer, T value)
{
    i64 size = sizeof(T);
    void* data = serializer->buffer.data + serializer->offset;
    serializer->offset += size;

    *(T*)data = value;
}

template<typename T>
static void serialize_write_array(Serializer* serializer, T* array, i64 count)
{
    i64 size = sizeof(T) * count;
    void* data = serializer->buffer.data + serializer->offset;
    serializer->offset += size;

    memcpy(data, array, size);
}

template<typename T>
static void serialize_read(Serializer* serializer, T* out_value)
{
    i64 size = sizeof(T);
    void* data = serializer->buffer.data + serializer->offset;
    serializer->offset += size;

    *out_value = *(T*)data;
}

template<typename T>
static void serialize_read_array(Serializer* serializer, T** out_array, i64 count)
{
    i64 size = sizeof(T) * count;
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

static void serialize_write(Serializer* serializer, String string)
{
    serialize_write(serializer, string.length);
    serialize_write_array(serializer, string.data, string.length);
}
static void serialize_read(Serializer* serializer, String* string)
{
    serialize_read(serializer, &string->length);
    serialize_read_array(serializer, &string->data, string->length);
}

static void serialize_write(Serializer* serializer, const SubTexture& subtexture)
{
    String subtexture_name;
    if (subtexture.texture_index != INDEX_INVALID)
    {
        subtexture_name = get_subtexture_name(subtexture);
    }
    serialize_write(serializer, subtexture_name);
}

static void serialize_read(Serializer* serializer, SubTexture* subtexture)
{
    String subtexture_name;
    serialize_read(serializer, &subtexture_name);
    if (subtexture_name.length > 0)
    {
        *subtexture = *get_subtexture(subtexture_name);
    }
}

static void serialize_write(Serializer* serializer, const Animator& animator)
{
    serialize_write(serializer, animator.current_animation);
    serialize_write(serializer, animator.requested_animation);

    serialize_write(serializer, animator.current_frame);
    serialize_write(serializer, animator.elapsed_time);
}

static void serialize_read(Serializer* serializer, Animator* animator)
{
    serialize_read(serializer, &animator->current_animation);
    serialize_read(serializer, &animator->requested_animation);

    serialize_read(serializer, &animator->current_frame);
    serialize_read(serializer, &animator->elapsed_time);
}

static void serialize_write(Serializer* serializer, const Collider_Polygon& polygon)
{
    serialize_write(serializer, polygon.point_count);
    serialize_write_array(serializer, polygon.points, polygon.point_count);
}

static void serialize_read(Serializer* serializer, Collider_Polygon* polygon)
{
    serialize_read(serializer, &polygon->point_count);
    serialize_read_array(serializer, &polygon->points, polygon->point_count);
}

static void serialize_write(Serializer* serializer, const Collider& collider)
{
    serialize_write(serializer, collider.type);
    switch (collider.type)
    {
        case ColliderType_AABB:     serialize_write(serializer, collider.aabb);     break;
        case ColliderType_Circle:   serialize_write(serializer, collider.circle);   break;
        case ColliderType_Polygon:  serialize_write(serializer, collider.polygon);  break;
    }
}

static void serialize_read(Serializer* serializer, Collider* collider)
{
    serialize_read(serializer, &collider->type);
    switch (collider->type)
    {
        case ColliderType_AABB:     serialize_read(serializer, &collider->aabb);     break;
        case ColliderType_Circle:   serialize_read(serializer, &collider->circle);   break;
        case ColliderType_Polygon:  serialize_read(serializer, &collider->polygon);  break;
    }
}

static void entity_serialize(Serializer* serializer, const Entity& entity)
{
    serialize_write(serializer, entity.id);

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

static void entity_deserialize(Serializer* serializer, Entity* entity, World* world)
{
    serialize_read(serializer, &entity->id);

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
    serializer.buffer = push_string(arena, MEGABYTES(5));

    serialize_write(&serializer, world.entity_max_count);
    serialize_write(&serializer, world.entity_count);

    for (i32 idx = 0; idx < world.entity_count; ++idx)
    {
        entity_serialize(&serializer, world.entity_array[idx]);
    }

    i64 unused_memory = serializer.buffer.length - serializer.offset;
    arena_pop(arena, unused_memory);

    return String{ serializer.buffer.data, serializer.offset };
}

World world_deserialize(String serialized)
{
    Serializer serializer;
    serializer.buffer = serialized;

    i32 entity_max_count;
    serialize_read(&serializer, &entity_max_count);
    World world = world_init(entity_max_count);

    serialize_read(&serializer, &world.entity_count);

    for (i32 idx = 0; idx < world.entity_count; ++idx)
    {
        Entity* entity = &world.entity_array[idx];
        entity_deserialize(&serializer, entity, &world);
    }

    world.free_list = nullptr;
    for (i32 idx = 0; idx < world.entity_count; ++idx)
    {
        Entity* entity = &world.entity_array[idx];
        if (!entity_flags_has_any(*entity, EntityFlag_Active))
        {
            entity->next = world.free_list;
            world.free_list = entity;
        }
    }

    return world;
}

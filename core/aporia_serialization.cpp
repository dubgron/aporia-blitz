#include "aporia_serialization.hpp"

#include "aporia_parser.hpp"
#include "aporia_utils.hpp"

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
    if (count > 0)
    {
        i64 size = sizeof(T) * count;
        void* data = serializer->buffer.data + serializer->offset;
        serializer->offset += size;

        memcpy(data, array, size);
    }
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
    if (count > 0)
    {
        i64 size = sizeof(T) * count;
        void* data = serializer->buffer.data + serializer->offset;
        serializer->offset += size;

        *out_array = arena_push_uninitialized<T>(serializer->arena, count);
        memcpy(*out_array, data, size);
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

    serialize_read(serializer, &entity->collider);
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

    serializer.arena = &world.arena;

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

//////////////////////////////////////////////////

static void serialize_text_write(StringList* builder, MemoryArena* arena, String name)
{
    builder->push_node(arena, name);
}

template<typename T>
static void serialize_text_write(StringList* builder, MemoryArena* arena, String name, T value)
{
    builder->push_node(arena, sprintf(arena, "% %", name, value));
}

template<typename T>
static void serialize_text_write(StringList* builder, MemoryArena* arena, String name, T value1, T value2)
{
    builder->push_node(arena, sprintf(arena, "% % %", name, value1, value2));
}

static void serialize_text_write(StringList* builder, MemoryArena* arena, String name, String string)
{
    builder->push_node(arena, sprintf(arena, "% \"%\"", name, string));
}

static void serialize_text_write(StringList* builder, MemoryArena* arena, String name, const Collider& collider)
{
    builder->push_node(arena, sprintf(arena, "% % ; %", name, (u32)collider.type, collider_type_to_string(collider.type)));
}

template<typename T>
static void serialize_text_write_as_hex(StringList* builder, MemoryArena* arena, String name, T value)
{
    ScratchArena temp = scratch_begin(arena);
    builder->push_node(arena, sprintf(arena, "% % ; %", name, to_hex(temp.arena, value), value));
    scratch_end(temp);
}

template<typename T>
static void serialize_text_write_as_hex(StringList* builder, MemoryArena* arena, String name, T value1, T value2)
{
    ScratchArena temp = scratch_begin(arena);
    builder->push_node(arena, sprintf(arena, "% % % ; % %", name, to_hex(temp.arena, value1), to_hex(temp.arena, value2), value1, value2));
    scratch_end(temp);
}

static void serialize_text_write_as_hex(StringList* builder, MemoryArena* arena, String name, Color color)
{
    ScratchArena temp = scratch_begin(arena);
    builder->push_node(arena, sprintf(arena, "% % ; % % % %", name, to_hex(temp.arena, *(u32*)&color), +color.r, +color.g, +color.b, +color.a));
    scratch_end(temp);
}

static void entity_serialize_to_text(MemoryArena* arena, StringList* builder, const Entity& entity)
{
    const Entity default_entity;

    ScratchArena temp = scratch_begin(arena);
    defer { scratch_end(temp); };

    serialize_text_write(builder, arena, "{");

    serialize_text_write(builder, arena, "  id", entity.id.index, entity.id.generation);

    if (entity.flags != default_entity.flags)
        serialize_text_write_as_hex(builder, arena, "  flags", entity.flags);

    if (entity.position != default_entity.position)
        serialize_text_write_as_hex(builder, arena, "  position", entity.position.x, entity.position.y);

    if (entity.z != default_entity.z)
        serialize_text_write_as_hex(builder, arena, "  z", entity.z);

    if (entity.rotation != default_entity.rotation)
        serialize_text_write_as_hex(builder, arena, "  rotation", entity.rotation);

    if (entity.center_of_rotation != default_entity.center_of_rotation)
        serialize_text_write_as_hex(builder, arena, "  center_of_rotation", entity.center_of_rotation.x, entity.center_of_rotation.y);

    if (entity.width != default_entity.width)
        serialize_text_write_as_hex(builder, arena, "  width", entity.width);

    if (entity.height != default_entity.height)
        serialize_text_write_as_hex(builder, arena, "  height", entity.height);

    if (entity.scale != default_entity.scale)
        serialize_text_write_as_hex(builder, arena, "  scale", entity.scale.x, entity.scale.y);

    if (entity.texture.texture_index != INDEX_INVALID)
    {
        String subtexture_name = get_subtexture_name(entity.texture);
        serialize_text_write(builder, arena, "  texture", subtexture_name);
    }

    if (entity.color != default_entity.color)
        serialize_text_write_as_hex(builder, arena, "  color", entity.color);

    if (entity.shader_id != default_entity.shader_id)
        serialize_text_write(builder, arena, "  shader_id", entity.shader_id);

    if (entity.animator.current_animation != default_entity.animator.current_animation)
    {
        serialize_text_write(builder, arena, "  animator {");

        serialize_text_write(builder, arena, "    current_animation", entity.animator.current_animation);
        serialize_text_write(builder, arena, "    requested_animation", entity.animator.requested_animation);

        serialize_text_write(builder, arena, "    current_frame", entity.animator.current_frame);
        serialize_text_write_as_hex(builder, arena, "    elapsed_time", entity.animator.elapsed_time);

        serialize_text_write(builder, arena, "  }");
    }

    if (entity.collider.type != ColliderType_None)
    {
        serialize_text_write(builder, arena, "  collider {");

        serialize_text_write(builder, arena, "    type", entity.collider);

        switch (entity.collider.type)
        {
            case ColliderType_AABB:
            {
                serialize_text_write_as_hex(builder, arena, "    base", entity.collider.aabb.base.x, entity.collider.aabb.base.y);
                serialize_text_write_as_hex(builder, arena, "    width", entity.collider.aabb.width);
                serialize_text_write_as_hex(builder, arena, "    height", entity.collider.aabb.height);
            }
            break;

            case ColliderType_Circle:
            {
                serialize_text_write_as_hex(builder, arena, "    base", entity.collider.circle.base.x, entity.collider.circle.base.y);
                serialize_text_write_as_hex(builder, arena, "    radius", entity.collider.circle.radius);
            }
            break;

            case ColliderType_Polygon:
            {
                serialize_text_write(builder, arena, "    point_count", entity.collider.polygon.point_count);

                if (entity.collider.polygon.point_count > 0)
                {
                    serialize_text_write(builder, arena, "    points");

                    for (i64 idx = 0; idx < entity.collider.polygon.point_count; ++idx)
                    {
                        v2 point = entity.collider.polygon.points[idx];
                        serialize_text_write_as_hex(builder, arena, "     ", point.x, point.y);
                    }
                }
            }
            break;
        }

        serialize_text_write(builder, arena, "  }");
    }

    serialize_text_write(builder, arena, "}");
}

String world_serialize_to_text(MemoryArena* arena, const World& world)
{
    MemoryArena temp = arena_init(MEGABYTES(20));

    StringList builder;
    serialize_text_write(&builder, &temp, "[world]");
    serialize_text_write(&builder, &temp, "entity_max_count", world.entity_max_count);
    serialize_text_write(&builder, &temp, "entity_count", world.entity_count);

    if (world.entity_count > 0)
    {
        serialize_text_write(&builder, &temp, "entity_array");
        for (i32 idx = 0; idx < world.entity_count; ++idx)
        {
            entity_serialize_to_text(&temp, &builder, world.entity_array[idx]);
        }
    }

    String result = builder.join(arena, "\n");
    arena_deinit(&temp);

    return result;
}

static Entity entity_deserialize_from_text(World* world, ParseTreeNode* field)
{
    Entity entity;

    for (ParseTreeNode* node = field->child_first; node; node = node->next)
    {
        if (node->name == "id")
        {
            get_value_from_field(node, &entity.id.index, 2);
        }
        else if (node->name == "flags")
        {
            get_value_from_field(node, &entity.flags);
        }
        else if (node->name == "position")
        {
            get_value_from_field(node, (f32*)&entity.position, 2);
        }
        else if (node->name == "z")
        {
            get_value_from_field(node, &entity.z);
        }
        else if (node->name == "rotation")
        {
            get_value_from_field(node, &entity.rotation);
        }
        else if (node->name == "center_of_rotation")
        {
            get_value_from_field(node, (f32*)&entity.center_of_rotation, 2);
        }
        else if (node->name == "width")
        {
            get_value_from_field(node, &entity.width);
        }
        else if (node->name == "height")
        {
            get_value_from_field(node, &entity.height);
        }
        else if (node->name == "scale")
        {
            get_value_from_field(node, &entity.scale[0], 2);
        }
        else if (node->name == "texture")
        {
            String subtexture_name;
            get_value_from_field(node, &subtexture_name);
            if (subtexture_name.length > 0)
            {
                entity.texture = *get_subtexture(subtexture_name);
            }
        }
        else if (node->name == "color")
        {
            if (node->child_count == 1 && node->child_first->value_flags & ValueFlag_Hex)
            {
                get_value_from_field(node, (u32*)&entity.color);
            }
            else
            {
                get_value_from_field(node, &entity.color.r, node->child_count);
            }
        }
        else if (node->name == "shader_id")
        {
            get_value_from_field(node, &entity.shader_id);
        }
        else if (node->name == "animator")
        {
            for (ParseTreeNode* anim_node = node->child_first; anim_node; anim_node = anim_node->next)
            {
                if (anim_node->name == "current_animation")
                {
                    String current_animation;
                    get_value_from_field(anim_node, &current_animation);
                    entity.animator.current_animation = push_string(&world->arena, current_animation);
                }
                else if (anim_node->name == "requested_animation")
                {
                    String requested_animation;
                    get_value_from_field(anim_node, &requested_animation);
                    entity.animator.requested_animation = push_string(&world->arena, requested_animation);
                }
                else if (anim_node->name == "current_frame")
                {
                    get_value_from_field(anim_node, &entity.animator.current_frame);
                }
                else if (anim_node->name == "elapsed_time")
                {
                    get_value_from_field(anim_node, &entity.animator.elapsed_time);
                }
            }
        }
        else if (node->name == "collider")
        {
            for (ParseTreeNode* coll_node = node->child_first; coll_node; coll_node = coll_node->next)
            {
                if (coll_node->name == "type")
                {
                    get_value_from_field(coll_node, (u8*)&entity.collider.type);
                    break;
                }
            }

            switch (entity.collider.type)
            {
                case ColliderType_AABB:
                {
                    for (ParseTreeNode* coll_node = node->child_first; coll_node; coll_node = coll_node->next)
                    {
                        if (coll_node->name == "base")
                        {
                            get_value_from_field(coll_node, (f32*)&entity.collider.aabb.base, 2);
                        }
                        else if (coll_node->name == "width")
                        {
                            get_value_from_field(coll_node, &entity.collider.aabb.width);
                        }
                        else if (coll_node->name == "height")
                        {
                            get_value_from_field(coll_node, &entity.collider.aabb.height);
                        }
                    }
                }
                break;

                case ColliderType_Circle:
                {
                    for (ParseTreeNode* coll_node = node->child_first; coll_node; coll_node = coll_node->next)
                    {
                        if (coll_node->name == "base")
                        {
                            get_value_from_field(coll_node, (f32*)&entity.collider.circle.base, 2);
                        }
                        else if (coll_node->name == "radius")
                        {
                            get_value_from_field(coll_node, &entity.collider.circle.radius);
                        }
                    }
                }
                break;

                case ColliderType_Polygon:
                {
                    for (ParseTreeNode* coll_node = node->child_first; coll_node; coll_node = coll_node->next)
                    {
                        if (coll_node->name == "point_count")
                        {
                            get_value_from_field(coll_node, &entity.collider.polygon.point_count);
                            break;
                        }
                    }

                    entity.collider.polygon.points = arena_push_uninitialized<v2>(&world->arena, entity.collider.polygon.point_count);

                    for (ParseTreeNode* coll_node = node->child_first; coll_node; coll_node = coll_node->next)
                    {
                        if (coll_node->name == "points")
                        {
                            get_value_from_field(coll_node, (f32*)entity.collider.polygon.points, entity.collider.polygon.point_count * 2);
                        }
                    }
                }
                break;
            }
        }
    }

    return entity;
}

World world_deserialize_from_text(String serialized)
{
    World world;

    MemoryArena temp = arena_init(MEGABYTES(40));
    ParseTreeNode* parsed = parse_from_memory(&temp, serialized);

    for (ParseTreeNode* category = parsed->child_first; category; category = category->next)
    {
        if (category->name == "world")
        {
            for (ParseTreeNode* field = category->child_first; field; field = field->next)
            {
                if (field->name == "entity_max_count")
                {
                    get_value_from_field(field, &world.entity_max_count);
                    break;
                }
            }

            world = world_init(world.entity_max_count);

            for (ParseTreeNode* field = category->child_first; field; field = field->next)
            {
                if (field->name == "entity_count")
                {
                    get_value_from_field(field, &world.entity_count);
                }
                else if (field->name == "entity_array")
                {
                    for (ParseTreeNode* node = field->child_first; node; node = node->next)
                    {
                        Entity entity = entity_deserialize_from_text(&world, node);
                        world.entity_array[entity.id.index] = entity;
                    }
                }
            }
        }
    }

    arena_deinit(&temp);
    return world;
}

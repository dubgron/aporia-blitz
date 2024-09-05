#include "aporia_entity.hpp"

#include "aporia_game.hpp"

bool operator==(EntityID id0, EntityID id1)
{
    return id0.index == id1.index && id0.generation == id1.generation;
}

bool entity_flags_has_all(const Entity& entity, EntityFlags flags)
{
    return (entity.flags & flags) == flags;
}

bool entity_flags_has_any(const Entity& entity, EntityFlags flags)
{
    return (entity.flags & flags) != 0;
}

void entity_flags_set(Entity* entity, EntityFlags flags)
{
    entity->flags |= flags;
}

void entity_flags_unset(Entity* entity, EntityFlags flags)
{
    entity->flags &= ~flags;
}

String entity_type_to_string(EntityType type)
{
    switch (type)
    {
        case Entity_None: return "None";
        case Entity_Player: return "Player";

        default: return "INVALID";
    }
}

void entity_adjust_size_to_texture(Entity* entity)
{
    get_subtexture_size(entity->texture, &entity->width, &entity->height);
}

Collider entity_collider_from_local_to_world(const Entity& entity)
{
    Collider result = entity.collider;
    switch (result.type)
    {
        case ColliderType_AABB:
        {
            v2 scaled_size = v2{ entity.width, entity.height } * entity.scale;
            v2 offset_from_center = scaled_size * entity.center_of_rotation;
            v2 base_offset = entity.position - offset_from_center;

            result.aabb.base += base_offset;
            result.aabb.width *= entity.scale.x;
            result.aabb.height *= entity.scale.y;
        }
        break;

        case ColliderType_Circle:
        {
            result.circle.base += entity.position;
            result.circle.radius *= max(entity.scale.x, entity.scale.y);
        }
        break;

        case ColliderType_Polygon:
        {
            f32 sin = std::sin(entity.rotation);
            f32 cos = std::cos(entity.rotation);

            v2 right_offset = v2{ cos, sin } * entity.scale.x;
            v2 up_offset = v2{ -sin, cos } * entity.scale.y;

            result.polygon.points = arena_push_uninitialized<v2>(&memory.frame, result.polygon.point_count);
            for (i64 idx = 0; idx < result.polygon.point_count; ++idx)
            {
                v2 local_position = entity.collider.polygon.points[idx];
                result.polygon.points[idx] = entity.position + (right_offset * local_position.x) + (up_offset * local_position.y);
            }
        }
        break;
    }
    return result;
}

Entity entity_lerp(const Entity& e0, const Entity& e1, f32 t)
{
    Entity result = e0;
    result.position = lerp(e0.position, e1.position, t);
    result.z = lerp(e0.z, e1.z, t);
    result.rotation = lerp(e0.rotation, e1.rotation, t);
    result.center_of_rotation = lerp(e0.center_of_rotation, e1.center_of_rotation, t);
    result.width = lerp(e0.width, e1.width, t);
    result.height = lerp(e0.height, e1.height, t);
    result.scale = lerp(e0.scale, e1.scale, t);
    result.color = lerp(e0.color, e1.color, t);
    return result;
}

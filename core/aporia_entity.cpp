#include "aporia_entity.hpp"

#include "aporia_game.hpp"

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

void entity_ajust_size_to_texture(Entity* entity)
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

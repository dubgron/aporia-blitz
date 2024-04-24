#pragma once

#include "aporia_animations.hpp"
#include "aporia_collision.hpp"
#include "aporia_shaders.hpp"
#include "aporia_textures.hpp"
#include "aporia_utils.hpp"

using EntityFlags = u64;
enum EntityFlag_ : EntityFlags
{
    EntityFlag_None             = 0x00,
    EntityFlag_Active           = 0x01,

    EntityFlag_Visible          = 0x02,
    EntityFlag_BlockingLight    = 0x04,

    EntityFlag_CollisionEnabled = 0x08,
};

struct EntityID
{
    i32 index = INDEX_INVALID;
    i32 generation = INDEX_INVALID;
};

struct Entity
{
    EntityID id;
    Entity* next = nullptr;

    EntityFlags flags = EntityFlag_Visible | EntityFlag_BlockingLight;

    v2 position{ 0.f };
    f32 z = 0.f;

    f32 rotation = 0.f;
    v2 center_of_rotation{ 0.f };

    f32 width = 0.f;
    f32 height = 0.f;
    v2 scale{ 1.f };

    SubTexture texture;
    Color color = Color::White;
    u32 shader_id = default_shader;

    Animator animator;

    Collider collider;
};

bool entity_flags_has_all(const Entity& entity, EntityFlags flags);
bool entity_flags_has_any(const Entity& entity, EntityFlags flags);
void entity_flags_set(Entity* entity, EntityFlags flags);
void entity_flags_unset(Entity* entity, EntityFlags flags);

void entity_ajust_size_to_texture(Entity* entity);

Collider entity_collider_from_local_to_world(const Entity& entity);

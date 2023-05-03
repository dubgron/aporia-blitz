#pragma once

#include "aporia_animation.hpp"
#include "aporia_shaders.hpp"
#include "aporia_textures.hpp"
#include "aporia_utils.hpp"

namespace Aporia
{
    enum EntityFlag : u64
    {
        EntityFlag_None             = 0x00,
        EntityFlag_Visible          = 0x01,
        EntityFlag_BlockingLight    = 0x02,
    };

    struct Entity
    {
        u64 index = 0;

        u64 flags = EntityFlag_Visible | EntityFlag_BlockingLight;

        v2 position{ 0.f };
        f32 z = 0.f;

        f32 rotation = 0.f;
        v2 center_of_rotation{ 0.f };

        f32 width = 0.f;
        f32 height = 0.f;
        v2 scale{ 1.f };

        const SubTexture* texture = nullptr;
        Color color;
        u32 shader_id = default_shader;

        Animator animator;

        f32 velocity = 0.f;
    };

    bool is_flag_set(const Entity& entity, EntityFlag flag);
    void set_flag(Entity& entity, EntityFlag flag);
    void unset_flag(Entity& entity, EntityFlag flag);

    void entity_ajust_size_to_texture(Entity& entity);
}

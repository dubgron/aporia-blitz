#pragma once

#include "aporia_string.hpp"
#include "aporia_textures.hpp"

namespace Aporia
{
    struct Entity;

    struct AnimationFrame
    {
        const SubTexture* texture = nullptr;
    };

    struct Animation
    {
        AnimationFrame* frames = nullptr;
        u64 frame_count = 0;
        f32 frame_length = 0.1f;
    };

    struct Animator
    {
        String current_animation;
        String requested_animation;

        u64 current_frame = 0;
        f32 elapsed_time = 0.f;
    };

    void animations_init(MemoryArena* arena);

    void load_animations(String filepath);

    void animation_tick(Entity& entity, f32 frame_time);
    void animation_request(Animator& animator, String animation_name);
}

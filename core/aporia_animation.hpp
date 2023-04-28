#pragma once

#include <vector>

#include "aporia_strings.hpp"
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
        String name;

        std::vector<AnimationFrame> frames;
        u64 current_frame = 0;
        f32 frame_length = 0.1f;
    };

    struct Animator
    {
        std::vector<Animation> animations;

        Animation* current_animation = nullptr;
        Animation* requested_animation = nullptr;

        f32 elapsed_time = 0.f;
    };

    void load_animations(std::string_view filepath);

    void animation_tick(Entity& entity, f32 frame_time);
    void animation_add(Animator& animator, String animation_name);

    void animation_request(Animator& animator, String animation_name);
}

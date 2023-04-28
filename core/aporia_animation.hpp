#pragma once

#include <vector>

#include "aporia_strings.hpp"
#include "aporia_textures.hpp"

namespace Aporia
{
    struct Entity;

    struct AnimationFrameData
    {
        const SubTexture* texture = nullptr;
    };

    struct Animation
    {
        String name;
        std::vector<AnimationFrameData> frames;
        u64 current_frame = 0;
        f32 frame_length = 0.1f;
    };

    struct Animator
    {
        Entity* owner = nullptr;
        std::vector<Animation> animations;
        Animation* current_animation = nullptr;
        Animation* requested_animation = nullptr;
        f32 elapsed_time = 0.f;
    };

    void load_animations(std::string_view filepath);

    void animator_update(Animator& animator, f32 frame_time);
    void animator_add_animation(Animator& animator, String anim_name);

    void animation_request(Animator& animator, String animation_name);
}

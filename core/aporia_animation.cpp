#include "aporia_animation.hpp"

#include "aporia_config.hpp"
#include "aporia_debug.hpp"
#include "aporia_entity.hpp"
#include "aporia_game.hpp"
#include "aporia_strings.hpp"
#include "aporia_textures.hpp"

namespace Aporia
{
    static constexpr u64 MAX_ANIMATIONS = 100;
    static constexpr u64 MAX_ANIMATIONS_PER_ENTITY = 10;

    static Animation* all_animations = nullptr;
    static u64 all_animations_count = 0;

    void animations_init(MemoryArena* arena)
    {
        all_animations = arena->push_zero<Animation>(MAX_ANIMATIONS);
        all_animations_count = 0;
    }

    // @TODO(dubgron): The arena should be parameterized in the future.
    void load_animations(String filepath)
    {
        ScratchArena temp = create_scratch_arena(&frame_arena);
        Config_Property* parsed_file = parse_config_from_file(temp.arena, filepath);

        for (const Config_Property* property = parsed_file; property; property = property->next)
        {
            if (property->category != "animations")
            {
                continue;
            }

            const String animation_name = property->field;
            const u64 frames_count = property->literals.node_count;

            Animation animation;
            animation.name = push_string(&persistent_arena, animation_name);
            animation.frames = persistent_arena.push<AnimationFrame>(frames_count);

            for (const StringNode* frame_node = property->literals.first; frame_node; frame_node = frame_node->next)
            {
                AnimationFrame frame;
                frame.texture = get_subtexture(frame_node->string);

                animation.frames[animation.frames_count] = frame;
                animation.frames_count += 1;
            }

            APORIA_ASSERT(all_animations_count < MAX_ANIMATIONS);
            all_animations[all_animations_count] = animation;
            all_animations_count += 1;
        }

        rollback_scratch_arena(temp);
    }

    void animation_tick(Entity& entity, f32 frame_time)
    {
        Animator& animator = entity.animator;
        Animation* animation = animator.current_animation;
        if (!animation)
        {
            animator.current_animation = animator.requested_animation;
        }

        if (animation)
        {
            animator.elapsed_time += frame_time;
            while (animator.elapsed_time >= animation->frame_length)
            {
                animator.elapsed_time -= animation->frame_length;

                if (animator.requested_animation)
                {
                    animator.current_animation = animator.requested_animation;
                    animator.requested_animation = nullptr;
                }

                animation->current_frame += 1;
                if (animation->current_frame > animation->frames_count - 1)
                {
                    animation->current_frame = 0;
                }

                entity.texture = animation->frames[animation->current_frame].texture;
            }
        }
    }

    // @TODO(dubgron): The arena should be parameterized in the future.
    void animation_add(Animator& animator, String animation_name)
    {
        // Init the animator
        if (animator.animations == nullptr)
        {
            animator.animations = persistent_arena.push<Animation>(MAX_ANIMATIONS_PER_ENTITY);
            animator.animations_count = 0;
        }

        for (u64 idx = 0; idx < all_animations_count; ++idx)
        {
            if (all_animations[idx].name == animation_name)
            {
                APORIA_ASSERT(animator.animations_count < MAX_ANIMATIONS_PER_ENTITY);
                animator.animations[animator.animations_count] = all_animations[idx];
                animator.animations_count += 1;
                break;
            }
        }
    }

    void animation_request(Animator& animator, String animation_name)
    {
        for (u64 idx = 0; idx < animator.animations_count; ++idx)
        {
            if (animator.animations[idx].name == animation_name)
            {
                animator.requested_animation = &animator.animations[idx];
                break;
            }
        }

        if (animator.requested_animation == nullptr)
        {
            APORIA_LOG(Error, "Failed to find animation '%'!", animation_name);
        }
    }
}

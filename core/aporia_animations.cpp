#include "aporia_animations.hpp"

#include "aporia_config.hpp"
#include "aporia_debug.hpp"
#include "aporia_entity.hpp"
#include "aporia_game.hpp"
#include "aporia_hash_table.hpp"
#include "aporia_string.hpp"
#include "aporia_textures.hpp"

namespace Aporia
{
    static constexpr u64 MAX_ANIMATIONS = 128;
    static HashTable<Animation> all_animations;

    static bool operator==(const Animation& animation1, const Animation& animation2)
    {
        return animation1.frames == animation2.frames
            && animation1.frame_count == animation2.frame_count
            && animation1.frame_length == animation2.frame_length;
    }

    void animations_init(MemoryArena* arena)
    {
        all_animations = hash_table_create<Animation>(arena, MAX_ANIMATIONS);
    }

    // @TODO(dubgron): The arena should be parameterized in the future.
    void load_animations(String filepath)
    {
        ScratchArena temp = scratch_begin();
        Config_Property* parsed_file = parse_config_from_file(temp.arena, filepath);

        for (const Config_Property* property = parsed_file; property; property = property->next)
        {
            if (property->category != "animations")
            {
                continue;
            }

            const String animation_name = push_string(&memory.persistent, property->field);
            const u64 frame_count = property->literals.node_count;

            Animation animation;
            animation.frames = arena_push_uninitialized<AnimationFrame>(&memory.persistent, frame_count);

            for (const StringNode* frame_node = property->literals.first; frame_node; frame_node = frame_node->next)
            {
                AnimationFrame frame;
                frame.texture = get_subtexture(frame_node->string);

                animation.frames[animation.frame_count] = frame;
                animation.frame_count += 1;
            }

            hash_table_insert(&all_animations, animation_name, animation);

            APORIA_ASSERT(*hash_table_find(&all_animations, animation_name) == animation);
        }

        scratch_end(temp);
    }

    void animation_tick(Entity& entity, f32 frame_time)
    {
        Animator& animator = entity.animator;

        if (animator.current_animation.is_empty())
        {
            return;
        }

        Animation* animation = hash_table_find(&all_animations, animator.current_animation);
        APORIA_ASSERT(animation);

        animator.elapsed_time += frame_time;
        while (animator.elapsed_time >= animation->frame_length)
        {
            animator.elapsed_time -= animation->frame_length;

            // The current frame is over. If other animation has been requested, play it.
            if (!animator.requested_animation.is_empty())
            {
                animation = hash_table_find(&all_animations, animator.requested_animation);
                APORIA_ASSERT(animation);

                animator.current_animation = animator.requested_animation;
                animator.requested_animation = String{};
            }

            // Increment the current frame and wrap, if necessary.
            animator.current_frame += 1;
            if (animator.current_frame > animation->frame_count - 1)
            {
                animator.current_frame = 0;
            }

            entity.texture = animation->frames[animator.current_frame].texture;
        }
    }

    void animation_request(Animator& animator, String animation)
    {
        if (animator.current_animation.is_empty())
        {
            animator.current_animation = animation;
        }
        else if (animator.current_animation != animation)
        {
            animator.requested_animation = animation;
        }
    }
}

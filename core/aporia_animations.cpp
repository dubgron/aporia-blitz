#include "aporia_animations.hpp"

#include "aporia_debug.hpp"
#include "aporia_entity.hpp"
#include "aporia_game.hpp"
#include "aporia_hash_table.hpp"
#include "aporia_parser.hpp"
#include "aporia_string.hpp"
#include "aporia_textures.hpp"

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
    defer { scratch_end(temp); };

    ParseTreeNode* parsed_file = parse_from_file(temp.arena, filepath);

    for (ParseTreeNode* node = parsed_file->child_first; node; node = node->next)
    {
        APORIA_ASSERT(node->type == ParseTreeNode_Category);
        if (node->name == "animations")
        {
            for (ParseTreeNode* animation_node = node->child_first; animation_node; animation_node = animation_node->next)
            {
                APORIA_ASSERT(animation_node->type == ParseTreeNode_Field);
                String animation_name = push_string(&memory.persistent, animation_node->name);
                u64 frame_count = animation_node->child_count;

                Animation animation;
                animation.frames = arena_push_uninitialized<AnimationFrame>(&memory.persistent, frame_count);

                for (ParseTreeNode* frame_node = animation_node->child_first; frame_node; frame_node = frame_node->next)
                {
                    APORIA_ASSERT(frame_node->type == ParseTreeNode_String);

                    AnimationFrame frame;
                    frame.texture = get_subtexture(frame_node->string_value);

                    animation.frames[animation.frame_count] = frame;
                    animation.frame_count += 1;
                }

                hash_table_insert(&all_animations, animation_name, animation);

                APORIA_ASSERT(*hash_table_find(&all_animations, animation_name) == animation);
            }
        }
    }
}

void animation_tick(Entity* entity, f32 frame_time)
{
    Animator* animator = &entity->animator;

    if (animator->current_animation.is_empty())
    {
        return;
    }

    Animation* animation = hash_table_find(&all_animations, animator->current_animation);
    APORIA_ASSERT(animation);

    animator->elapsed_time += frame_time;
    while (animator->elapsed_time >= animation->frame_length)
    {
        animator->elapsed_time -= animation->frame_length;

        // The current frame is over. If other animation has been requested, play it.
        if (!animator->requested_animation.is_empty())
        {
            animation = hash_table_find(&all_animations, animator->requested_animation);
            APORIA_ASSERT(animation);

            animator->current_animation = animator->requested_animation;
            animator->requested_animation = String{};
        }

        // Increment the current frame and wrap it around, if necessary.
        animator->current_frame += 1;
        if (animator->current_frame >= animation->frame_count)
        {
            animator->current_frame = 0;
        }

        entity->texture = *animation->frames[animator->current_frame].texture;
    }
}

void animation_request(Animator* animator, String animation)
{
    if (animator->current_animation.is_empty())
    {
        animator->current_animation = animation;
    }
    else if (animator->current_animation != animation)
    {
        animator->requested_animation = animation;
    }
}
